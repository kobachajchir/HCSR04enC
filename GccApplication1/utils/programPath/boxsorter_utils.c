/*
 * boxSorter.c
 *
 * Created: 26-Mar-25 9:34:08 AM
 *  Author: kobac
 */ 

#include "../../main.h"
#include "../../types/boxTypes.h"
#include "../../types/sorterSystemTypes.h"
#include "../../types/ultrasonicDetectorType.h"
#include "../../types/configType.h"
#include "boxsorter_utils.h"
#include <stdio.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

/**
 * @brief Inicializa el detector ultrasónico.
 *
 * Configura el puntero al sensor ultrasónico y al sensor IR asociado.
 *
 * @param [out] hcsr04Detector Estructura del detector ultrasónico.
 * @param [in] sensor_ultra Puntero al sensor ultrasónico.
 * @param [in] sensor_IR Puntero al sensor IR.
 */
void initDetector(Ultrasonic_Detector_t* hcsr04Detector, ultrasonic_t* sensor_ultra, TCRT_t* sensor_IR){
	hcsr04Detector->sensor = &ultraSensor;
	hcsr04Detector->sensor_IR = &sensor_IR;
	hcsr04Detector->flags.byte = 0;
	NIBBLEH_SET_STATE(hcsr04Detector->flags, ULTRADET_SENSOR_IDLE);;
}

/**
 * @brief Inicializa los servos utilizados por el sistema.
 *
 * Asigna ángulo inicial y pines de control. Los servos son almacenados en un arreglo global.
 */
inline void initServos(void){ //Funcion solo estructural, es inline
	initServo(&servoA, 0, SERVOA_PIN, SERVO_IDLE_ANGLE);
	initServo(&servoB, 1, SERVOB_PIN, SERVO_IDLE_ANGLE);
	//initServo(&servoC, 2, SERVOC_PIN, SERVO_IDLE_ANGLE); //Recordar que tenemos que habilitar este cuando pongamos el servo
	servosArray[0] = &servoA;
	servosArray[1] = &servoB;
	servosArray[2] = &servoC;
	current_servo = 0;
	//printf("Init servos\n");
}

/**
 * @brief Inicializa las salidas del sistema de clasificación.
 *
 * Carga configuración desde EEPROM si existe, o aplica valores por defecto.
 * Asocia pines y marca todas las salidas como listas (READY).
 */
void initOutputs(){
	initServos();
	if(existConfig()){
		loadConfigurationRAM(&currentConfig);
		salidaA.boxType = currentConfig.salidaA;
		salidaB.boxType = currentConfig.salidaB;
		salidaC.boxType = currentConfig.salidaC;
		if(DEBUG_FLAGS_EEPROM){
			printf_P(PSTR("Config cargada: Salida 0 = %u, Salida 1 = %u, Salida 2 = %u\n"),
			salidaA.boxType, salidaB.boxType, salidaC.boxType);
		}
	}else{
		salidaA.boxType = OUTPUT_A_DEFAULT_BOX_TYPE;
		salidaB.boxType = OUTPUT_B_DEFAULT_BOX_TYPE;
		salidaC.boxType = OUTPUT_C_DEFAULT_BOX_TYPE;
		if(DEBUG_FLAGS_SORTER){
			printf_P(PSTR("SYS SORTER DEBUG - Sin configuracion guardada\n"));
		}
	}
	salidaA.actuator_pin = SERVOA_PIN;
	salidaA.sensor_pin = IR_A.pin;
	salidaA.flags.byte = 0;
	salidaB.actuator_pin = SERVOB_PIN;
	salidaB.sensor_pin = IR_B.pin;
	salidaB.flags.byte = 0;
	salidaC.actuator_pin = SERVOC_PIN;
	salidaC.sensor_pin = IR_C.pin;
	salidaC.flags.byte = 0;
	SET_FLAG(salidaA.flags, OUTPUT_READY);
	SET_FLAG(salidaB.flags, OUTPUT_READY);
	SET_FLAG(salidaC.flags, OUTPUT_READY);
	//printf("Init outputs\n");
}

/**
 * @brief Inicializa el sistema de clasificación (Sorter).
 *
 * Asocia rangos de altura de cajas, salidas y configura estado inicial.
 *
 * @param [out] SystemSorter Puntero al sistema de clasificación a inicializar.
 */
void initSorter(sorter_system_t* SystemSorter){
	static box_height_range_matrix_t local_box_ranges = {
		.box_size_a = { BOX_A_MIN_MM, BOX_A_MAX_MM },
		.box_size_b = { BOX_B_MIN_MM, BOX_B_MAX_MM },
		.box_size_c = { BOX_C_MIN_MM, BOX_C_MAX_MM }
	};
	SystemSorter->box_ranges = &local_box_ranges;
	SystemSorter->outputs[0] = &salidaA;
	SystemSorter->outputs[1] = &salidaB;
	SystemSorter->outputs[2] = &salidaC;
	if(DEBUG_FLAGS_SORTER){
		printf_P(PSTR("SYS SORTER DEBUG - Matriz de dimensiones inicializada\n"));
		printf_P(PSTR("SYS SORTER DEBUG - Salidas agregadas\n"));
	}
}

/**
 * @brief Clasifica una caja en función de su altura medida.
 *
 * Determina si es tipo A, B, C o descartada, comparando contra los rangos predefinidos.
 *
 * @param [in] distance_mm Distancia medida por el sensor ultrasónico.
 * @param [in] SystemSorter Sistema de clasificación que contiene los rangos.
 * @retval box_type_t Tipo de caja detectada.
 */
box_type_t classify_box(uint8_t distance_mm, sorter_system_t* SystemSorter)
{
	// Si la distancia es mayor o igual a la distancia de reposo (pared)
	if (distance_mm >= DETECTION_IDLE_DISTANCE_MM) {
		if (DEBUG_FLAGS_SORTER) {
			printf_P(PSTR("No hay caja presente (pared detectada a %u mm)\n"), distance_mm);
		}
		return NO_BOX;
	}

	// Calculamos la altura de la caja como la diferencia con la pared
	uint8_t altura_mm = DETECTION_IDLE_DISTANCE_MM - distance_mm;

	if (altura_mm < DETECTION_THRESHOLD_MIN_MM) {
		if (DEBUG_FLAGS_SORTER) {
			printf_P(PSTR("Caja demasiado baja (%u mm), descartada\n"), altura_mm);
		}
		return BOX_DISCARDED;
	}

	// Comparamos contra los rangos definidos
	if (altura_mm >= SystemSorter->box_ranges->box_size_a.min_height_mm &&
	altura_mm <= SystemSorter->box_ranges->box_size_a.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf_P(PSTR("Caja tipo A (%u mm)\n"), altura_mm);
		}
		return BOX_SIZE_A;
	}
	else if (altura_mm >= SystemSorter->box_ranges->box_size_b.min_height_mm &&
	altura_mm <= SystemSorter->box_ranges->box_size_b.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf_P(PSTR("Caja tipo B (%u mm)\n"), altura_mm);
		}
		return BOX_SIZE_B;
	}
	else if (altura_mm >= SystemSorter->box_ranges->box_size_c.min_height_mm &&
	altura_mm <= SystemSorter->box_ranges->box_size_c.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf_P(PSTR("Caja tipo C (%u mm)\n"), altura_mm);
		}
		return BOX_SIZE_C;
	}

	// Si no entró en ningún rango, se descarta
	if (DEBUG_FLAGS_SORTER) {
		printf_P(PSTR("Caja fuera de rango (%u mm), descartada\n"), altura_mm);
	}
	return BOX_DISCARDED;
}

/**
 * @brief Tarea que ejecuta el control principal del sensor ultrasónico.
 *
 * Se encarga de disparar el sensor, verificar el eco, clasificar la caja,
 * actualizar estadísticas y manejar errores por timeout.
 *
 * @param [in,out] ultraDetector Estructura de control del sensor ultrasónico.
 * @param [in,out] sorter Sistema de clasificación.
 */
void ultraSensorTask(Ultrasonic_Detector_t* ultraDetector, sorter_system_t* sorter)
{
	// 1. Si está habilitado, el trigger está permitido y hay solicitud de emisión
	if (ULTRASONIC_ENABLE &&
		ultraDetector->sensor->TRIGGER_ALLOWED &&
		EMIT_TRIGGER &&
		IS_FLAG_SET(ultraDetector->flags, ULTRADET_ZONE_TRCT_U_DETECTING))
	{
		if (ultrasonic_start(ultraDetector->sensor)) {
			ULTRASONIC_ENABLE = 0;
			EMIT_TRIGGER = 0;
			ultraDetector->sensor->TRIGGER_ALLOWED = 0;
		} else {
			if (DEBUG_FLAGS_SORTER)
				printf_P(PSTR("ErrorInitHCSR04\n"));
			EMIT_FAILED = 1;
			EMIT_TRIGGER = 0;
		}
	}

	// 2. Actualiza máquina de estados del sensor
	ultrasonic_update(ultraDetector->sensor);

	// 3. Si la medición finalizó exitosamente
	if (ultraDetector->sensor->state == ULTRA_DONE && ultraDetector->sensor->NEW_RESULT) {
		ultraDetector->sensor->NEW_RESULT = 0;

		box_type_t tipo;

		if (ultraDetector->sensor->distance_mm > DETECTION_IDLE_DISTANCE_MM) {
			tipo = NO_BOX;
			SET_FLAG(ultraDetector->flags, ULTRADET_ZONE_ULTRA_CLEAR);

			if (NIBBLEH_GET_STATE(ultraDetector->flags) == ULTRADET_SENSOR_WAITING_CLEAR) {
				NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_IDLE);
				if (DEBUG_FLAGS_SORTER)
					printf_P(PSTR("Sensor liberado de nuevo\n"));
			}

			NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_IDLE);
		} else {
			if (NIBBLEH_GET_STATE(ultraDetector->flags) == ULTRADET_SENSOR_IDLE) {
				NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_DETECTING);
				tipo = classify_box(ultraDetector->sensor->distance_mm, sorter);

				if (tipo != NO_BOX) {
					if (DEBUG_FLAGS_SORTER) {
						printf_P(PSTR("Tipo detectado: %u - Salida 0: %u, 1: %u, 2: %u\n"),
								tipo, salidaA.boxType, salidaB.boxType, salidaC.boxType);
					}

					if (tipo != BOX_DISCARDED) {
						output_t* outputs[] = { &salidaA, &salidaB, &salidaC };

						for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
							if (outputs[i]->boxType == tipo) {
								SET_FLAG(outputs[i]->flags, OUTPUT_BUSY);
								if (DEBUG_FLAGS_SORTER)
									printf_P(PSTR("Salida %c busy\n"), 'A' + i);
								break;
							}
						}
					} else if (DEBUG_FLAGS_SORTER) {
						printf_P(PSTR("BOX_DISCARDED\n"));
					}

					// Actualizar estadísticas
					switch (tipo) {
						case BOX_SIZE_A:
						case BOX_SIZE_B:
						case BOX_SIZE_C:
							sorter->stats.total_by_type_array[tipo - 1]++;
							if (DEBUG_FLAGS_SORTER)
								printf_P(PSTR("BOX_SIZE_%c\n"), 'A' + (tipo - 1));
							break;

						case BOX_DISCARDED:
							sorter->stats.total_discarded++;
							if (DEBUG_FLAGS_SORTER)
								printf_P(PSTR("BOX_DISCARDED\n"));
							break;

						default:
							if (DEBUG_FLAGS_SORTER)
								printf_P(PSTR("Tipo no reconocido\n"));
							break;
					}

					sorter->stats.total_measured++;
					if (DEBUG_FLAGS_SORTER)
						printf_P(PSTR("Contadas: %u\n"), sorter->stats.total_measured);
				}

				NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_WAITING_CLEAR);
			}
		}

		ultrasonic_init_flags(ultraDetector->sensor);
		ultraDetector->sensor->state = ULTRA_IDLE;
		ULTRASONIC_ENABLE = 1;
	}

	// 4. Timeout por falta de eco
	if (VEINTEMS_PASSED) {
		if (DEBUG_FLAGS)
			printf_P(PSTR("HCSR04 perdio ECHO\n"));

		VEINTEMS_PASSED = 0;
		WAITING_ECHO = 0;
		ultraDetector->sensor->TIMEDOUT = 1;
		ultraDetector->sensor->state = ULTRA_TIMEOUT;

		if (ultrasonic_timeout_clear(ultraDetector->sensor, DEBUG_FLAGS ? true : false) && DEBUG_FLAGS)
			printf_P(PSTR("LIB DEBUG - HCSR04 TMDOUT Cleared\n"));

		ULTRASONIC_ENABLE = 1;
	}
}


/**
 * @brief Tarea que procesa los sensores IR.
 *
 * Realiza lectura de cada sensor, detecta cajas, y activa los servos correspondientes.
 *
 * @param [in,out] sorter Sistema de clasificación.
 */
void irSensorsTask(sorter_system_t * sorter){
	// IR A
	if (IS_FLAG_SET(IR_A.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_A.flags, TCRT_NEW_VALUE)) {
		CLEAR_FLAG(IR_A.flags, TCRT_NEW_VALUE);
		tcrt_read(&IR_A);
	}
	tcrt_is_box_detected(&IR_A); // Solo actualiza estados
	if (NIBBLEH_GET_STATE(IR_A.flags) == TCRT_COUNTED) {
		if (IS_FLAG_SET(salidaA.flags, OUTPUT_READY) && IS_FLAG_SET(salidaA.flags, OUTPUT_BUSY)) {
			CLEAR_FLAG(salidaA.flags, OUTPUT_READY);
			SET_FLAG(servoA.flags, SERVO_PUSH);
			servoA.state_time = 0;
			if (DEBUG_FLAGS_SORTER) {
				printf_P(PSTR("SERVO_PUSH A\n"));
				printf_P(PSTR("Pushed servo A and zeroed state time\n"));
			}
		}
		NIBBLEH_SET_STATE(IR_A.flags, TCRT_STATUS_IDLE);
		if (DEBUG_FLAGS_SORTER) {
			printf_P(PSTR("Detecto en IR A\n"));
		}
	}

	// IR B
	if (IS_FLAG_SET(IR_B.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_B.flags, TCRT_NEW_VALUE)) {
		CLEAR_FLAG(IR_B.flags, TCRT_NEW_VALUE);
		tcrt_read(&IR_B);
	}
	tcrt_is_box_detected(&IR_B);
	if (NIBBLEH_GET_STATE(IR_B.flags) == TCRT_COUNTED) {
		if (IS_FLAG_SET(salidaB.flags, OUTPUT_READY) && IS_FLAG_SET(salidaB.flags, OUTPUT_BUSY)) {
			CLEAR_FLAG(salidaB.flags, OUTPUT_READY);
			SET_FLAG(servoB.flags, SERVO_PUSH);
			servoB.state_time = 0;
			if (DEBUG_FLAGS_SORTER) {
				printf_P(PSTR("Pushed servo B and zeroed state time\n"));
			}
		}
		NIBBLEH_SET_STATE(IR_B.flags, TCRT_STATUS_IDLE);
		if (DEBUG_FLAGS_SORTER) {
			printf_P(PSTR("Detecto en IR B\n"));
		}
	}

	// IR C
	if (IS_FLAG_SET(IR_C.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_C.flags, TCRT_NEW_VALUE)) {
		CLEAR_FLAG(IR_C.flags, TCRT_NEW_VALUE);
		tcrt_read(&IR_C);
	}
	tcrt_is_box_detected(&IR_C);
	if (NIBBLEH_GET_STATE(IR_C.flags) == TCRT_COUNTED) {
		if (IS_FLAG_SET(salidaC.flags, OUTPUT_READY) && IS_FLAG_SET(salidaC.flags, OUTPUT_BUSY)) {
			CLEAR_FLAG(salidaC.flags, OUTPUT_READY);
			SET_FLAG(servoC.flags, SERVO_PUSH);
			servoC.state_time = 0;
			if (DEBUG_FLAGS_SORTER) {
				printf_P(PSTR("Pushed servo C and zeroed state time\n"));
			}
		}
		NIBBLEH_SET_STATE(IR_C.flags, TCRT_STATUS_IDLE);
		if (DEBUG_FLAGS_SORTER) {
			printf_P(PSTR("Detecto en IR C\n"));
		}
	}

	// IR U (ultrasonido - sin salida asociada)
	if (IS_FLAG_SET(IR_U.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_U.flags, TCRT_NEW_VALUE)) {
		CLEAR_FLAG(IR_U.flags, TCRT_NEW_VALUE);
		tcrt_read(&IR_U);
	}
	tcrt_is_box_detected(&IR_U);

	if (NIBBLEH_GET_STATE(IR_U.flags) == TCRT_READ && !IS_FLAG_SET(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING)) {
		SET_FLAG(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING);
	}
	else if (NIBBLEH_GET_STATE(IR_U.flags) == TCRT_COUNTED && IS_FLAG_SET(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING)) {
		CLEAR_FLAG(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING);
		NIBBLEH_SET_STATE(IR_U.flags, TCRT_STATUS_IDLE);
	}
	else if (!IS_FLAG_SET(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING)) {
		NIBBLEH_SET_STATE(IR_U.flags, TCRT_STATUS_IDLE);
	}
}

/**
 * @brief Función placeholder para futuras tareas relacionadas con los servos.
 *
 * Actualmente vacía.
 */
void servosTask() {

}

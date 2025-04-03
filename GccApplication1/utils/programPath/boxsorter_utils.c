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
#include "boxsorter_utils.h"
#include <stdio.h>
#include <stdbool.h>

void initDetector(Ultrasonic_Detector_t* hcsr04Detector, ultrasonic_t* sensor_ultra, TCRT_t* sensor_IR){
	hcsr04Detector->sensor = &ultraSensor;
	hcsr04Detector->sensor_IR = &sensor_IR;
	hcsr04Detector->flags.byte = 0;
	NIBBLEH_SET_STATE(hcsr04Detector->flags, ULTRADET_SENSOR_IDLE);
	printf("Init hcsr04Detector, sensor idle\n");
}

inline void initServos(void){
	initServo(&servoA, 0, SERVOA_PIN, SERVO_IDLE_ANGLE);
	initServo(&servoB, 1, SERVOB_PIN, SERVO_IDLE_ANGLE);
	//initServo(&servoC, 2, SERVOC_PIN, SERVO_IDLE_ANGLE); //Recordar que tenemos que habilitar este cuando pongamos el servo
	servosArray[0] = &servoA;
	servosArray[1] = &servoB;
	servosArray[2] = &servoC;
	current_servo = 0;
	//printf("Init servos\n");
}

void initOutputs(){
	initServos();
	//TODO Fn para leer EEPROM
	if(OUTPUT_A_HAS_CONFIG){
		printf("Salida A find setting\n");
	}else{
		printf("Salida A no setting\n");
		salidaA.boxType = OUTPUT_A_DEFAULT_BOX_TYPE;
	}
	if(OUTPUT_B_HAS_CONFIG){
		printf("Salida B find setting\n");
	}else{
		printf("Salida B no setting\n");
		salidaB.boxType = OUTPUT_B_DEFAULT_BOX_TYPE;
		
	}
	if(OUTPUT_C_HAS_CONFIG){
		printf("Salida C find setting\n");
	}else{
		printf("Salida C no setting\n");
		salidaC.boxType = OUTPUT_C_DEFAULT_BOX_TYPE;
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
		printf("SYS SORTER DEBUG - Matriz de dimensiones iniciada\n");
		printf("SYS SORTER DEBUG - Salidas agregadas\n");
	}
}


box_type_t classify_box(uint8_t distance_mm, sorter_system_t* SystemSorter)
{
	// Si la distancia es mayor o igual a la distancia de reposo (pared)
	if (distance_mm >= DETECTION_IDLE_DISTANCE_MM) {
		if (DEBUG_FLAGS_SORTER) {
			printf("No hay caja presente (pared detectada a %u mm)\n", distance_mm);
		}
		return NO_BOX;
	}

	// Calculamos la altura de la caja como la diferencia con la pared
	uint8_t altura_mm = DETECTION_IDLE_DISTANCE_MM - distance_mm;

	if (altura_mm < DETECTION_THRESHOLD_MIN_MM) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja demasiado baja (%u mm), descartada\n", altura_mm);
		}
		return BOX_DISCARDED;
	}

	// Comparamos contra los rangos definidos
	if (altura_mm >= SystemSorter->box_ranges->box_size_a.min_height_mm &&
	altura_mm <= SystemSorter->box_ranges->box_size_a.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja tipo A (%u mm)\n", altura_mm);
		}
		return BOX_SIZE_A;
	}
	else if (altura_mm >= SystemSorter->box_ranges->box_size_b.min_height_mm &&
	altura_mm <= SystemSorter->box_ranges->box_size_b.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja tipo B (%u mm)\n", altura_mm);
		}
		return BOX_SIZE_B;
	}
	else if (altura_mm >= SystemSorter->box_ranges->box_size_c.min_height_mm &&
	altura_mm <= SystemSorter->box_ranges->box_size_c.max_height_mm) {
		if (DEBUG_FLAGS_SORTER) {
			printf("Caja tipo C (%u mm)\n", altura_mm);
		}
		return BOX_SIZE_C;
	}

	// Si no entró en ningún rango, se descarta
	if (DEBUG_FLAGS_SORTER) {
		printf("Caja fuera de rango (%u mm), descartada\n", altura_mm);
	}
	return BOX_DISCARDED;
}



void ultraSensorTask(Ultrasonic_Detector_t* ultraDetector, sorter_system_t * sorter)
{
	// 1. Si está habilitado, el trigger está permitido y hay solicitud de emisión
	if (ULTRASONIC_ENABLE && ultraDetector->sensor->TRIGGER_ALLOWED && EMIT_TRIGGER && IS_FLAG_SET(ultraDetector->flags, ULTRADET_ZONE_TRCT_U_DETECTING)) //tiene que estar detectando en ul U para que pueda sensar ultrasonido
	{
		if (ultrasonic_start(ultraDetector->sensor))
		{
			if (DEBUG_FLAGS)
			{
				printf("InitHCSR04\n");
			}

			ULTRASONIC_ENABLE = 0;     // Desactivar para esperar fin de medición
			EMIT_TRIGGER = 0;          // Limpiar bandera de emisión
			ultraDetector->sensor->TRIGGER_ALLOWED = 0;  // Inhabilitar trigger hasta que se permita de nuevo
		}
		else
		{
			if (DEBUG_FLAGS)
			{
				printf("ErrorInitHCSR04\n");
			}
			EMIT_FAILED = 1;
			EMIT_TRIGGER = 0;
		}
	}

	// 2. Actualiza máquina de estados del sensor
	ultrasonic_update(ultraDetector->sensor);

	// 3. Si la medición finalizó exitosamente
	if (ultraDetector->sensor->state == ULTRA_DONE && ultraDetector->sensor->NEW_RESULT)
	{
		//printf("HCSR04 Dist[mm] %lu\n", (uint32_t)ultrasonic_get_distance(ultraDetector->sensor));
		ultraDetector->sensor->NEW_RESULT = 0;

		box_type_t tipo;

		// Si la distancia es mayor al límite de zona activa -> no hay caja
		if (ultraDetector->sensor->distance_mm > DETECTION_IDLE_DISTANCE_MM)
		{
			tipo = NO_BOX;
			SET_FLAG(ultraDetector->flags, ULTRADET_ZONE_ULTRA_CLEAR);

			// Si estaba esperando que se libere, volvemos a IDLE
			if (NIBBLEH_GET_STATE(ultraDetector->flags) == ULTRADET_SENSOR_WAITING_CLEAR)
			{
				NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_IDLE);
				if (DEBUG_FLAGS_SORTER) {
					printf("Sensor liberado de nuevo\n");
				}
			}

			NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_IDLE); // Momentáneo, se puede sacar si no se requiere más
		}
		else
		{
			// Si el sensor ya está libre (estado IDLE), se procede a clasificar la caja
			if (NIBBLEH_GET_STATE(ultraDetector->flags) == ULTRADET_SENSOR_IDLE) {
				// Hay algo en la zona
				NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_DETECTING);
				tipo = classify_box(ultraDetector->sensor->distance_mm, sorter);

				if (tipo != NO_BOX)
				{
					if(salidaA.boxType == tipo){
						SET_FLAG(salidaA.flags, OUTPUT_BUSY); //Setear como busy para que cuando detecte patee
						printf("Salida A busy\n");
					}else if(salidaB.boxType == tipo){
						SET_FLAG(salidaB.flags, OUTPUT_BUSY);
						printf("Salida B busy\n");
					}else if(salidaC.boxType == tipo){
						SET_FLAG(salidaC.flags, OUTPUT_BUSY);
						printf("Salida C busy\n");
					}else{
						printf("BOX_DISCARDED\n");
					}
					switch (tipo)
					{
						case BOX_SIZE_A:
						printf("BOX_SIZE_A\n");
						sorter->stats.total_by_type_array[0]++;
						break;
						case BOX_SIZE_B:
						printf("BOX_SIZE_B\n");
						sorter->stats.total_by_type_array[1]++;
						break;
						case BOX_SIZE_C:
						printf("BOX_SIZE_C\n");
						sorter->stats.total_by_type_array[2]++;
						break;
						case BOX_DISCARDED:
						printf("BOX_DISCARDED\n");
						sorter->stats.total_discarded++;
						break;
						default:
						printf("Tipo no reconocido\n");
						break;
					}
					sorter->stats.total_measured++;
					printf("Contadas: %u\n", sorter->stats.total_measured);
					// TODO: Aggregar cola para poder manejar las cajas
				}

				// Cambio de estado para esperar que se libere nuevamente
				NIBBLEH_SET_STATE(ultraDetector->flags, ULTRADET_SENSOR_WAITING_CLEAR);
			}
		}

		ultrasonic_init_flags(ultraDetector->sensor); // Reinicia las flags del sensor
		ultraDetector->sensor->state = ULTRA_IDLE;   // Reinicia el estado del sensor
		ULTRASONIC_ENABLE = 1; // Habilita nueva medición
	}


	// 4. Si pasó el tiempo de espera y no hubo ECHO
	if (VEINTEMS_PASSED)
	{
		if (DEBUG_FLAGS)
		{
			printf("HCSR04 perdio ECHO\n");
		}
		VEINTEMS_PASSED = 0;
		WAITING_ECHO = 0;

		ultraDetector->sensor->TIMEDOUT = 1;
		ultraDetector->sensor->state = ULTRA_TIMEOUT;

		if (ultrasonic_timeout_clear(ultraDetector->sensor, DEBUG_FLAGS ? true : false) && DEBUG_FLAGS)
		{
			printf("LIB DEBUG - HCSR04 TMDOUT Cleared\n");
		}

		ULTRASONIC_ENABLE = 1;
	}
}

void irSensorsTask(sorter_system_t * sorter){
	
	//printf("IR A: %u\n", IR_A.ADCConvertedValue);	
	//printf("IR B: %u\n", IR_A.ADCConvertedValue);	
	//printf("IR C: %u\n", IR_C.ADCConvertedValue);
	//Para ir_U, hay que hacer otro debug, no tiene una salida asignada
	if(IS_FLAG_SET(IR_A.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_A.flags, TCRT_NEW_VALUE)){ //Cada 10 ms se activa
		CLEAR_FLAG(IR_A.flags, TCRT_NEW_VALUE);
		tcrt_read(&IR_A);
	}
	if(tcrt_is_box_detected(&IR_A)){
		if(NIBBLEH_GET_STATE(IR_A.flags) == TCRT_COUNTED){ //Ya detecto y paso a contarlo
			if(IS_FLAG_SET(salidaA.flags, OUTPUT_READY) && IS_FLAG_SET(salidaA.flags, OUTPUT_BUSY)){
				CLEAR_FLAG(salidaA.flags, OUTPUT_READY);
				SET_FLAG(servoA.flags, SERVO_PUSH);
				servoA.state_time = 0;
				printf("Pushed servo A and zeroed state time\n");
			}
			NIBBLEH_SET_STATE(IR_A.flags, TCRT_STATUS_IDLE);
			printf("Detecto en IR A\n");
		}
	}
	if(IS_FLAG_SET(IR_B.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_B.flags, TCRT_NEW_VALUE)){ //Cada 20 ms se activa
		CLEAR_FLAG(IR_B.flags, TCRT_NEW_VALUE);
		tcrt_read(&IR_B);
	}
	if(tcrt_is_box_detected(&IR_B)){
		if(NIBBLEH_GET_STATE(IR_B.flags) == TCRT_COUNTED){ //Ya detecto y paso a contarlo
			if(IS_FLAG_SET(salidaB.flags, OUTPUT_READY) && IS_FLAG_SET(salidaB.flags, OUTPUT_BUSY)){
				CLEAR_FLAG(salidaB.flags, OUTPUT_READY);
				SET_FLAG(servoB.flags, SERVO_PUSH);
				servoB.state_time = 0;
				printf("Pushed servo B and zeroed state time\n");
			}
			NIBBLEH_SET_STATE(IR_B.flags, TCRT_STATUS_IDLE);
			printf("Detecto en IR B\n");
		}
	}
	if(IS_FLAG_SET(IR_C.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_C.flags, TCRT_NEW_VALUE)){ //Cada 20 ms se activa
		CLEAR_FLAG(IR_C.flags, TCRT_NEW_VALUE);
		tcrt_read(&IR_C);
	}
	if(tcrt_is_box_detected(&IR_C)){
		if(NIBBLEH_GET_STATE(IR_C.flags) == TCRT_COUNTED){ //Ya detecto y paso a contarlo
			if(IS_FLAG_SET(salidaC.flags, OUTPUT_READY) && IS_FLAG_SET(salidaC.flags, OUTPUT_BUSY)){
				CLEAR_FLAG(salidaC.flags, OUTPUT_READY);
				SET_FLAG(servoC.flags, SERVO_PUSH);
				servoC.state_time = 0;
				printf("Pushed servo C and zeroed state time\n");
			}
			NIBBLEH_SET_STATE(IR_C.flags, TCRT_STATUS_IDLE);
			printf("Detecto en IR C\n");
		}
	}
	if(IS_FLAG_SET(IR_U.flags, TCRT_ENABLED) && IS_FLAG_SET(IR_U.flags, TCRT_NEW_VALUE)){ //Cada 20 ms se activa IR U del ultrasonido, no tiene salida
		CLEAR_FLAG(IR_U.flags, TCRT_NEW_VALUE);
		tcrt_read(&IR_U);
	}
	if(tcrt_is_box_detected(&IR_U)){
		// La caja está detectada y es un flanco de subida (recibimos la caja)
		if(NIBBLEH_GET_STATE(IR_U.flags) == TCRT_READ && !IS_FLAG_SET(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING)){
			// Solo entra aquí si la caja está detectada y no está marcando la zona como detectada
			SET_FLAG(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING);
			//printf("IR U detecto\n");
		}
		// La caja está detectada y es un flanco de bajada (la caja salió)
		else if(NIBBLEH_GET_STATE(IR_U.flags) == TCRT_COUNTED && IS_FLAG_SET(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING)){
			// Esto garantiza que solo se imprimirá cuando la caja haya salido
			//printf("IR U detecto: la caja salió\n");
			// Limpiar la zona de detección, ya que la caja salió
			CLEAR_FLAG(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING);
			NIBBLEH_SET_STATE(IR_U.flags, TCRT_STATUS_IDLE);  // Restablecer el estado
		}
		} else {
			// Si no hay caja y estábamos detectando una, limpiar las banderas
			if(IS_FLAG_SET(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING)){
				// Ya no hay caja y se había detectado previamente
				CLEAR_FLAG(hcsr04Detector.flags, ULTRADET_ZONE_TRCT_U_DETECTING);
				NIBBLEH_SET_STATE(IR_U.flags, TCRT_STATUS_IDLE);  // Poner el sensor en estado idle
				//printf("IR U Idle again\n");
			}
	}
}

void servosTask() {

}

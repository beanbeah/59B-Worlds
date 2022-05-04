#include "main.h"

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	//base Blue
	Motor FL1(FL1Port,E_MOTOR_GEARSET_06, true, E_MOTOR_ENCODER_DEGREES);
	Motor FL2(FL2Port,E_MOTOR_GEARSET_06, false, E_MOTOR_ENCODER_DEGREES);
	Motor FL3(FL3Port,E_MOTOR_GEARSET_06, true, E_MOTOR_ENCODER_DEGREES);
	Motor FR1(FR1Port,E_MOTOR_GEARSET_06, false, E_MOTOR_ENCODER_DEGREES);
	Motor FR2(FR2Port,E_MOTOR_GEARSET_06, true, E_MOTOR_ENCODER_DEGREES);
	Motor FR3(FR3Port,E_MOTOR_GEARSET_06, false, E_MOTOR_ENCODER_DEGREES);

	//Lift green
	Motor armLeft(armLeftPort,E_MOTOR_GEARSET_36,true,E_MOTOR_ENCODER_DEGREES);
	Motor armRight(armRightPort,E_MOTOR_GEARSET_36,false,E_MOTOR_ENCODER_DEGREES);

	//penumatic init
	ADIDigitalOut clamp(clampPort);
	ADIDigitalOut batch(batchPort);
	ADIDigitalOut needle(needlePort);

	//sensor init
	ADIAnalogIn armPotentiometer(armPotentiometerPort);
	Rotation encoderR(encdRPort);
	Rotation encoderS(encdSPort);
	Imu imu(imuPort);
	encoderR.reset_position();
	encoderS.reset_position();
	imu.reset();

	// Mech tasks
	Task sensorTask(sensors, (void*)"PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Sensor Task");
	Task armControlTask(armControl, (void*)"PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Arm Control Task");
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
	double start = millis();
	setOffset(-90);
	baseTurn(-90);
	delay(50);
	Task controlTask(PPControl, (void*)"PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "PP Task");
	Task odometryTask(Odometry, (void*)"PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Odom Task");

	//get AWP
	enableBase(true,true);
	delay(250);
	baseMove(-20);
	waitPP(1000);
	baseMove(12);
	waitPP(1000);
	baseTurn(calcBaseTurn(30.5, 26, false));
	waitTurn(1200);
	setArmPos(1);
	baseMove(10);
	waitPP(1000);
	delay(150);
	setBatchState(false);
	delay(1000);
	setNeedleState(false);
	delay(300);
	baseMove(11.7);
	waitPP(1000);

	//intake L-shape rings
	// driverArmPos(0);
	// baseMove(-12);
	// waitPP(1000);
	// baseTurn(180,0.1);
	// waitTurn(2000);
	// setMaxRPMV(400);
	// baseMove(-90);
	// waitPP(5000);
	printf("Ended in %.2f seconds\n", (millis()-start)/1000);

}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	Motor FL1(FL1Port);
  Motor FL2(FL2Port);
  Motor FL3(FL3Port);
  Motor FR1(FR1Port);
  Motor FR2(FR2Port);
  Motor FR3(FR3Port);
	Motor armLeft(armLeftPort);
	Motor armRight(armRightPort);

	FL1.set_brake_mode(E_MOTOR_BRAKE_BRAKE);
	FL2.set_brake_mode(E_MOTOR_BRAKE_BRAKE);
	FL3.set_brake_mode(E_MOTOR_BRAKE_BRAKE);
	FR1.set_brake_mode(E_MOTOR_BRAKE_BRAKE);
	FR2.set_brake_mode(E_MOTOR_BRAKE_BRAKE);
	FR3.set_brake_mode(E_MOTOR_BRAKE_BRAKE);


	Controller master(E_CONTROLLER_MASTER);
	Controller partner(E_CONTROLLER_PARTNER);

	int armPos = 0, goalPos = 0, tiltPos = 0;
	bool tankDrive = true, init = true,partnerOveride = false;
	int tick = 0;
	while(true) {
		double left, right;
		if(master.get_digital_new_press(DIGITAL_Y)) tankDrive = !tankDrive;

		if(tankDrive) {
			left = master.get_analog(ANALOG_LEFT_Y);
			right = master.get_analog(ANALOG_RIGHT_Y);
		} else {
			double power = master.get_analog(ANALOG_LEFT_Y);
			double turn = master.get_analog(ANALOG_RIGHT_X);
			left = power + turn;
			right = power - turn;
		}

		FL1.move(left);
		FL2.move(left);
		FL3.move(left);
		FR1.move(right);
		FR2.move(right);
		FR3.move(right);

		if (armClampState){
			if(init){
				master.rumble("...");
				init = false;
			}
			if (master.get_digital_new_press(DIGITAL_L1) || partner.get_digital_new_press(DIGITAL_R1)) driverArmUp();
			else if (master.get_digital_new_press(DIGITAL_L2) || partner.get_digital_new_press(DIGITAL_R2)) driverArmDown();
			if(tick%500==0)master.rumble("...");
		} else {
			init = true;
			if (master.get_digital_new_press(DIGITAL_L1) || partner.get_digital_new_press(DIGITAL_R1)) driverArmUp();
			else if(master.get_digital_new_press(DIGITAL_L2) || partner.get_digital_new_press(DIGITAL_R2)) driverArmDown();
		}

		if(master.get_digital_new_press(DIGITAL_X)) toggleArmClampState();
		if(master.get_digital_new_press(DIGITAL_R1)) toggleNeedleState();
		if(master.get_digital_new_press(DIGITAL_R2)) toggleBatchState();

		master.print(2,0,"meong");
		tick++;
		delay(5);
  	}
}

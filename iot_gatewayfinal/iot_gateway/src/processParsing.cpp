#include "application.h"
#include <iostream>
#include "MuParser/muParser.h"

void loadParsingMethods()
{
}

double MySqr(double a_fVal) { return a_fVal * a_fVal; }
int testMuParser()
{
	try
	{
		double var_a = 1;
		mu::Parser p;
		p.DefineVar("a", &var_a);
		p.DefineFun("MySqr", MySqr);
		p.SetExpr("MySqr(a)*_pi+min(10,a)");
		

		for (std::size_t a = 0; a < 100; ++a)
		{
			var_a = a; // Change value of variable a
			std::cout << "muParser:" << a << " " << p.Eval() << std::endl;
		}
	}
	catch (mu::Parser::exception_type &e)
	{
		std::cout << e.GetMsg() << std::endl;
	}

	return 0;
}

const char *formulaSettings = "formulaSettings";
const char *ai_1_formula = "ai_1_formula";
const char *ai_2_formula = "ai_2_formula";
const char *do_1_formula = "do_1_formula";
const char *do_1_mode_conf = "do_1_mode";
const char *do_1_manual_conf = "do_1_manual";
const char *do_1_persistance_off = "do_1_persistance_off";
const char *do_1_persistance_on = "do_1_persistance_on";
 
double getModbusValue(double modbusId, double parameterId) {
	double value = -1;
	
	for (uint32_t mx = 0; mx < NO_OF_MODBUS_METERS; ++mx){
        ModbusDevice *deviceObj = &liveObj.modbusDevices[mx];
		if(deviceObj->isActive && deviceObj->id == modbusId){ 
			for (uint32_t rx = 0; rx < deviceObj->noOfRegisters; ++rx){
                ApplicationRegister *mbRegister = &deviceObj->mb_register[rx]; 
				if(mbRegister->id == parameterId){
					String strValue = mbRegister->getValue();
					value = strValue.toDouble();
					break; 
				}
			}
		}
	}

    return value;
}


double getSnmpValue(double modbusId, double parameterId) {
	double value = -1;
	
	for (uint32_t mx = 0; mx < NO_OF_SNMP_DEVICES; ++mx){
        SNMP_Device *deviceObj = &liveObj.snmpDevices[mx];
		if(deviceObj->isActive && deviceObj->id == modbusId){ 
			for (uint32_t rx = 0; rx < deviceObj->noOfRegisters; ++rx){
                SnmpRegister *mbRegister = &deviceObj->snmpRegisters[rx]; 
				if(mbRegister->id == parameterId){
					String strValue = mbRegister->getValue();
					value = strValue.toDouble();
					break; 
				}
			}
		}
	}
	
    return value;
}

String computeExpression(String expression, double* result)
{ 
	String error = "";
	mu::Parser parser;
	double value = 0;
	try
	{
		if(expression != nullptr && expression.length() > 0){
			double AI_1_HW = liveObj.ai_1_adc;
			double AI_2_HW = liveObj.ai_2_adc; 
			double AI_1_VALUE = liveObj.ai_1_value;
			double AI_2_VALUE = liveObj.ai_2_value;
			double do_1_value = liveObj.do_1_value; 
			double do_1_mode = liveObj.do_1_mode;
			double do_1_manual = liveObj.do_1_manual;

			parser.DefineVar("AI_1_HW", &AI_1_HW);
			parser.DefineVar("AI_2_HW", &AI_2_HW);
			parser.DefineVar("AI_1_VALUE", &AI_1_VALUE);
			parser.DefineVar("AI_2_VALUE", &AI_2_VALUE);
			parser.DefineVar("DO_1_VALUE", &do_1_value);
			parser.DefineVar("DO_1_MODE", &do_1_mode);
			parser.DefineVar("DO_1_MAUAL", &do_1_manual);

			parser.DefineFun("getModbusValue", getModbusValue);       // Register the 'Add' function
			parser.DefineFun("getSnmpValue", getSnmpValue); // Register the 'Multiply' function

			parser.SetExpr(expression.c_str());

			value = parser.Eval();
		}
		if(result != nullptr){
			*result = value;
		}

	}
	catch (mu::Parser::exception_type &e)
	{
		error = e.GetMsg().c_str();
		std::cout << e.GetMsg() << std::endl;
	}
	
	parser.ClearVar(); 
  	parser.ClearConst(); // Clear all constants
  	parser.SetExpr("");  // Clear the expression

	return error;
}

void saveFormulas()
{
	String mqttString;
	JsonDocument mqttDoc;
	Serial.println("Saving formulas...");

	mqttDoc[ai_1_formula] = liveObj.ai_1_formula;
	mqttDoc[ai_2_formula] = liveObj.ai_2_formula;
	mqttDoc[do_1_formula] = liveObj.do_1_formula;
	mqttDoc[do_1_mode_conf] = liveObj.do_1_mode;
	mqttDoc[do_1_manual_conf] = liveObj.do_1_manual;

	mqttDoc[do_1_persistance_off] = liveObj.do_1_persistance_off;
	mqttDoc[do_1_persistance_on] = liveObj.do_1_persistance_on;

	serializeJson(mqttDoc, mqttString);

	preferences.begin(formulaSettings, false);
	preferences.putString(formulaSettings, mqttString);
	preferences.end();
}

void loadformulas()
{

	preferences.begin(formulaSettings, false);
	bool isRestored = false;

	if (preferences.isKey(formulaSettings))
	{
		String formulaString = preferences.getString(formulaSettings);
		JsonDocument formulaDoc;
		DeserializationError error = deserializeJson(formulaDoc, formulaString.c_str());
		if (!error)
		{
			liveObj.ai_1_formula = formulaDoc[ai_1_formula].as<String>();
			liveObj.ai_2_formula = formulaDoc[ai_2_formula].as<String>();
			liveObj.do_1_formula = formulaDoc[do_1_formula].as<String>();
			liveObj.do_1_manual = formulaDoc[do_1_manual_conf].as<int>();
			liveObj.do_1_mode = formulaDoc[do_1_mode_conf].as<int>();
			liveObj.do_1_mode = formulaDoc[do_1_mode_conf].as<int>();

			liveObj.do_1_persistance_off = formulaDoc[do_1_persistance_off].as<int>();
			liveObj.do_1_persistance_on = formulaDoc[do_1_persistance_on].as<int>();

			isRestored = true;
		}
	}

	if (isRestored == false)
	{
		liveObj.ai_1_formula = "";
		liveObj.ai_2_formula = "";
		liveObj.do_1_formula = "";
		liveObj.do_1_manual = 0;
		liveObj.do_1_mode = 0;
		liveObj.do_1_persistance_off = 0;
		liveObj.do_1_persistance_on = 0;
	}

	// Close Preferences
	preferences.end();
}


void procesFormulas(){
	double result;
	String error;
	
	liveObj.ai_1_adc = analogRead(ANALOG_1_IP);
	liveObj.ai_2_adc = analogRead(ANALOG_2_IP);

	if(liveObj.ai_1_formula.length() == 0){
		liveObj.ai_1_value = liveObj.ai_1_adc;
	}else{
		error = computeExpression(liveObj.ai_1_formula, &result);
		if(error.length()){ 
			liveObj.ai_1_value = -1;
		}else{
			liveObj.ai_1_value = result;
		}
	}

	
	if(liveObj.ai_2_formula.length() == 0){
		liveObj.ai_2_value = liveObj.ai_2_adc;
	}else{
		error = computeExpression(liveObj.ai_2_formula, &result);
		if(error.length()){ 
			liveObj.ai_2_value = -1;
		}else{
			liveObj.ai_2_value = result;
		}
	}


	if(liveObj.do_1_mode){
		 liveObj.do_1_value = liveObj.do_1_manual; 
		digitalWrite(RELAY_OP, liveObj.do_1_value == 0? LOW:HIGH);
	}
	else if(liveObj.do_1_formula.length() == 0){
		 liveObj.do_1_value = 0;
		 digitalWrite(RELAY_OP, LOW);
	}else{
		error = computeExpression(liveObj.do_1_formula, &result);
		if(error.length()){  
			liveObj.do_1_value = 0;
		}else{
			liveObj.do_1_value = result > 0; 
		}
	}
	if(digitalRead(RELAY_OP) == liveObj.do_1_value){ 
		digitalWrite(RELAY_OP, liveObj.do_1_value == 0? LOW:HIGH);
		liveObj.do_1_persistance_time = millis();
	}else{
		if(liveObj.do_1_value == 0 && get_abs_diffrence(liveObj.do_1_persistance_time, millis()) >= liveObj.do_1_persistance_off){
			digitalWrite(RELAY_OP, LOW); 
		}else if(liveObj.do_1_value == 1 && get_abs_diffrence(liveObj.do_1_persistance_time, millis()) >= liveObj.do_1_persistance_on){
			digitalWrite(RELAY_OP, HIGH); 			
		}
	}
}

#include "application.h"

#include <stdlib.h>
#include <math.h>
#include <stdexcept>

#include "descent.h"
#include "controls.h"
#include "Controller.h"
#include "ddio.h"
#include "ddio_common.h"

//TODO: Missing Win32 controller source
char Ctltext_AxisBindings[][16] = {
	"", "X-axis", "Y-axis", "Z-axis", "R-axis", "U-axis", "V-axis"
};
char Ctltext_BtnBindings[][16] = {
	"", "btn1", "btn2", "btn3", "btn4", "btn5", "btn6", "btn7", "btn8", "btn9", "btn10", "btn11",
		"btn12", "btn13", "btn14", "btn15", "btn16", "btn17", "btn18", "btn19", "btn20", "btn21", "btn22",
		"btn23", "btn24", "btn25", "btn26", "btn27", "btn28", "btn29", "btn30", "btn31", "btn32"
};
char Ctltext_PovBindings[][16] = {
	"","pov-U", "pov-R", "pov-D", "pov-L"
};


gameWinController::gameWinController(int num_funcs, ct_function* funcs, char* remote_adr)
	: gameController(num_funcs, funcs)
{
}

gameWinController::~gameWinController()
{
}

void gameWinController::suspend()
{
}

void gameWinController::resume()
{
}

void gameWinController::poll()
{
}

void gameWinController::flush()
{
}

ct_config_data gameWinController::get_controller_value(ct_type type_req)
{
	return 0;
}

void gameWinController::set_controller_function(int id, const ct_type* type, ct_config_data value, const ubyte* flags)
{
}

void gameWinController::get_controller_function(int id, ct_type* type, ct_config_data* value, ubyte* flags)
{
}

void gameWinController::enable_function(int id, bool enable)
{
}

bool gameWinController::get_packet(int id, ct_packet* packet, ct_format alt_format)
{
	return false;
}

float gameWinController::get_axis_sensitivity(ct_type axis_type, ubyte axis)
{
	return 0;
}

void gameWinController::set_axis_sensitivity(ct_type axis_type, ubyte axis, float val)
{
}

int gameWinController::assign_function(ct_function* fn)
{
	return 0;
}

void gameWinController::mask_controllers(bool joystick, bool mouse)
{
}

const char* gameWinController::get_binding_text(ct_type type, ubyte ctrl, ubyte bind)
{
	return "TEMP";
}

int gameWinController::get_mouse_raw_values(int* x, int* y)
{
	return 0;
}

unsigned gameWinController::get_joy_raw_values(int* x, int* y)
{
	return 0;
}

void gameWinController::set_controller_deadzone(int ctl, float deadzone)
{
}

bool gameWinController::enum_controllers(char* remote_adr)
{
	return false;
}

void gameWinController::assign_element(int id, ct_element* elem)
{
}

sbyte gameWinController::get_axis_controller(ubyte axis)
{
	return 0;
}

sbyte gameWinController::get_button_controller(ubyte btn)
{
	return 0;
}

sbyte gameWinController::get_pov_controller(ubyte pov)
{
	return 0;
}

float gameWinController::get_axis_value(sbyte controller, ubyte axis, ct_format format, bool invert)
{
	return 0;
}

float gameWinController::get_button_value(sbyte controller, ct_format format, ubyte button)
{
	return 0;
}

float gameWinController::get_pov_value(sbyte controller, ct_format format, ubyte pov_number, ubyte pov)
{
	return 0;
}

float gameWinController::get_key_value(int key, ct_format format)
{
	return 0;
}

void gameWinController::parse_ctl_file(int devnum, const char* ctlname)
{
}

void gameWinController::extctl_getpos(int id)
{
}

void gameWinController::extctl_geteval(int id)
{
}

void gameWinController::mouse_geteval()
{
}

gameController* CreateController(int num_funcs, ct_function* funcs, char* remote_ip)
{
	return new gameWinController(num_funcs, funcs, remote_ip);
}

void DestroyController(gameController* ctl)
{
	if (ctl)
		delete ctl;
}


/*
test_controller_class::test_controller_class(int numControls, ControlMatrixEntry* controlMatrix)
{
	int i;
	Init();

	for (i = 0; i < numControls; i++)
	{
		BindFromDefaultEntry(&controlMatrix[i]);
	}

	ControlsSuspended = false;
	LastReadDelta = 1.0;
	LastReadTime = -1;
	ControlFrameTime = 0;
	ResetControls();
}

void test_controller_class::ResetControls()
{
	ddio_KeyFlush();
	ddio_MouseQueueFlush();

	InitializeControls(false, false);
	InitializeControls(UseJoystick, UseMouse);
}

void test_controller_class::UpdateControllerStates()
{
	int i;
	if (!ControlsSuspended)
	{
		longlong time = timer_GetMSTime();
		if (LastReadTime == -1)
		{
			ControlLastReadTime = time;
			ControlFrameTime = 0.0;
			return;
		}
		LastReadTime = time;
		ControlFrameTime += LastReadDelta;
		if (ControlFrameTime >= 0.05)
			ControlFrameTime = 0;

		for (i = 0; i < NumDevices; i++)
		{
			if (Controllers[i].StickNum == -2)
				ReadMouseState();
			else if (Controllers[i].StickNum >= 0)
				ReadJoyState(Controllers[i].StickNum);
		}
	}
}

bool test_controller_class::ReadControlState(int bindNum, ControlState* state, uint readMode)
{
	ubyte deviceNum;
	ubyte buttonNum;
	uint readType;
	int i;
	float heldTime;

	readType = readMode;
	if (readMode == 0)
		readType = Bindings[bindNum].BindingMode;
	state->ReadMode = readType;

	ControlReadTime = timer_GetTime();
	state->Flags = 0;
	heldTime = 0;

	if (Bindings[bindNum].Initialized)
	{
		for (i = 0; i < 2; i++)
		{
			deviceNum = Bindings[bindNum].DeviceNumber[i];
			buttonNum = Bindings[bindNum].Binds[i];

			if (deviceNum == 255 || Controllers[deviceNum].StickNum == -3)
				continue;

			switch (Bindings[bindNum].Types[i])
			{
			case CONTROL_HAT:
				heldTime = CheckHatState(deviceNum, readType, 0, readMode & 255);
				break;
			case CONTROL_KEYBOARD:
				if (buttonNum != 0)
				{
					heldTime = CheckKeyState(buttonNum & 255, readType);
					if (DDIO_key_state[buttonNum & 255])
						state->Flags |= 1;
				}
				break;
			case CONTROL_MOUSEAXIS:
				state->Flags |= 2;
			case CONTROL_AXIS:
				heldTime = CheckAxisState(deviceNum, buttonNum, readType, 0);

				if (Bindings[bindNum].Flags[i] & 1) //check invert state
				{
					if (readType == BMODE_AXIS)
						heldTime = -heldTime;
					else if (readType != BMODE_DOWNSTATE)
					{
						if (heldTime != 0)
							heldTime = 0;
						else
							heldTime = 1.0;
					}
				}
				break;
			case CONTROL_MOUSEBUTTON:
				state->Flags |= 2;
			case CONTROL_JOYBUTTON:
				heldTime = CheckButtonState(deviceNum, readType, buttonNum);
				break;
			case CONTROL_JOYHAT2:
			case CONTROL_JOYHAT3:
			case CONTROL_JOYHAT4:
				heldTime = CheckHatState(deviceNum, readType, Bindings[bindNum].Types[i] - 6, readMode & 255);
				break;
			default:
				heldTime = 0.0;
				break;
			}
		}
	}

	if (heldTime != 0)
		state->Flags |= 1;

	state->TimeDown = heldTime;
	return true;
}

int test_controller_class::CheckForControl(int controlType)
{
	int i, j;
	int hatNum;
	int bit;
	int deviceCode;
	int ret = 0;
	float value;

	switch (controlType)
	{
	case CONTROL_AXIS:
	{
		float testRange;

		for (i = NUM_FIXED_CONTROLLERS; i < NumDevices; i++)
		{
			deviceCode = (i & 255) + 65280;
			if (Controllers[i].AxesMax & 32)
			{
				if (Controllers[i].Sensitivity[5] >= 1.5)
					testRange = .95f;
				else if (Controllers[i].Sensitivity[5] >= 1.0)
					testRange = .8f;
				else
					testRange = Controllers[i].Sensitivity[5] * .5f;

				value = CheckAxisState(i, 6, 1, 0.0);

				if (fabs(value) > testRange)
					ret = deviceCode * 65536 + 6;
			}
			else if (Controllers[i].AxesMax & 16)
			{
				if (Controllers[i].Sensitivity[4] >= 1.5)
					testRange = .95f;
				else if (Controllers[i].Sensitivity[4] >= 1.0)
					testRange = .8f;
				else
					testRange = Controllers[i].Sensitivity[4] * .5f;

				value = CheckAxisState(i, 5, 1, 0.0);

				if (fabs(value) > testRange)
					ret = deviceCode * 65536 + 5;
			}
			else if (Controllers[i].AxesMax & 8)
			{
				if (Controllers[i].Sensitivity[3] >= 1.5)
					testRange = .95f;
				else if (Controllers[i].Sensitivity[3] >= 1.0)
					testRange = .8f;
				else
					testRange = Controllers[i].Sensitivity[3] * .5f;

				value = CheckAxisState(i, 4, 1, 0.0);

				if (fabs(value) > testRange)
					ret = deviceCode * 65536 + 4;
			}
			else if (Controllers[i].AxesMax & 4)
			{
				if (Controllers[i].Sensitivity[2] >= 1.5)
					testRange = .95f;
				else if (Controllers[i].Sensitivity[2] >= 1.0)
					testRange = .8f;
				else
					testRange = Controllers[i].Sensitivity[2] * .5f;

				value = CheckAxisState(i, 3, 1, 0.0);

				if (fabs(value) > testRange)
					ret = deviceCode * 65536 + 3;
			}
			else if (Controllers[i].AxesMax & 2)
			{
				if (Controllers[i].Sensitivity[1] >= 1.5)
					testRange = .95f;
				else if (Controllers[i].Sensitivity[1] >= 1.0)
					testRange = .8f;
				else
					testRange = Controllers[i].Sensitivity[1] * .5f;

				value = CheckAxisState(i, 2, 1, 0.0);

				if (fabs(value) > testRange)
					ret = deviceCode * 65536 + 2;
			}
			else if (Controllers[i].AxesMax & 1)
			{
				if (Controllers[i].Sensitivity[0] >= 1.5)
					testRange = .95f;
				else if (Controllers[i].Sensitivity[0] >= 1.0)
					testRange = .8f;
				else
					testRange = Controllers[i].Sensitivity[0] * .5f;

				value = CheckAxisState(i, 1, 1, 0.0);

				if (fabs(value) > testRange)
					ret = deviceCode * 65536 + 1;
			}
		}

		return ret;
	}
	break;
	case CONTROL_HAT:
	case CONTROL_JOYHAT2:
	case CONTROL_JOYHAT3:
	case CONTROL_JOYHAT4:
		if (controlType == CONTROL_HAT)
			hatNum = 0;
		else hatNum = controlType - 6;

		bit = 64 << (hatNum & 31);

		for (i = NUM_FIXED_CONTROLLERS; i < NumDevices; i++)
		{
			deviceCode = (i & 255) - 256;
			if (Controllers[i].AxesMax & bit && CheckHatState(i, 2, hatNum, 64))
				ret = deviceCode * 65536 + 64;
			if (Controllers[i].AxesMax & bit && CheckHatState(i, 2, hatNum, 192))
				ret = deviceCode * 65536 + 192;
			if (Controllers[i].AxesMax & bit && CheckHatState(i, 2, hatNum, 128))
				ret = deviceCode * 65536 + 128;
			if (Controllers[i].AxesMax & bit && CheckHatState(i, 2, hatNum, 0))
				ret = deviceCode * 65536;
		}
		break;
	case CONTROL_JOYBUTTON:
		for (i = NUM_FIXED_CONTROLLERS; i < NumDevices; i++)
		{
			for (j = 0; j < Controllers[i].NumButtons; j++)
			{
				if (JoystickStates[i].ButtonState[j])
					return ((i & 255) + 65280) * 65536 + (j + 1 & 255);
			}
		}
		//TODO: this is a bit weird
		return 4294901760;
	case CONTROL_KEYBOARD:
		return ddio_KeyInKey() & 65535;
	case CONTROL_MOUSEAXIS:
		if (Controllers[CONTROLLER_MOUSE].AxesMax & 32 && fabs(CheckAxisState(1, 6, 1, 0.0)) > 0.5)
			return 4278255622;
		if (Controllers[CONTROLLER_MOUSE].AxesMax & 16 && fabs(CheckAxisState(1, 5, 1, 0.0)) > 0.5)
			return 4278255621;
		if (Controllers[CONTROLLER_MOUSE].AxesMax & 8 && fabs(CheckAxisState(1, 4, 1, 0.0)) > 0.5)
			return 4278255620;
		if (Controllers[CONTROLLER_MOUSE].AxesMax & 4 && fabs(CheckAxisState(1, 3, 1, 0.0)) > 0.5)
			return 4278255619;
		if (Controllers[CONTROLLER_MOUSE].AxesMax & 2 && fabs(CheckAxisState(1, 2, 1, 0.0)) > 0.5)
			return 4278255618;
		if (Controllers[CONTROLLER_MOUSE].AxesMax & 1 && fabs(CheckAxisState(1, 1, 1, 0.0)) > 0.5)
			return 4278255617;
		break;
	case CONTROL_MOUSEBUTTON:
		for (i = 0; i < 32; i++)
		{
			bit = ddio_MouseBtnUpCount(i);
			if (bit != 0)
				return (i + 1 & 255) - 16711680;
		}
		return 4294901760;
	}

	return ret;
}

void test_controller_class::SetInitialized(int num, bool initialized)
{
	Bindings[num].Initialized = initialized;
}

void test_controller_class::GetControlBinding(int num, int* bindTypes, int* binds, ubyte* inverts) const
{
	//[ISB] lint
	if (!bindTypes || !binds || !inverts)
		return;

	bindTypes[0] = Bindings[num].Types[0];
	bindTypes[1] = Bindings[num].Types[1];
	*binds = Bindings[num].Binds[0] + (Bindings[num].Binds[1] << 8) + (Bindings[num].DeviceNumber[0] << 16) + (Bindings[num].DeviceNumber[1] << 24);
	inverts[0] = Bindings[num].Flags[0];
	inverts[1] = Bindings[num].Flags[1];
}

void test_controller_class::SetControlBinding(int bindingNum, int* bindTypes, int binds, ubyte* inverts)
{
	//[ISB] lint
	if (!bindTypes || !inverts)
		return;

	ControlBinding bind;

	if (bindingNum < 255)
	{
		bind.Types[0] = bindTypes[0];
		if (bind.Types[0] == CONTROL_KEYBOARD)
			bind.DeviceNumber[0] = 0;
		else
			bind.DeviceNumber[0] = binds >> 16;
		bind.Types[1] = bindTypes[1];
		if (bind.Types[1] == CONTROL_KEYBOARD)
			bind.DeviceNumber[1] = 0;
		else
			bind.DeviceNumber[1] = binds >> 24;
		bind.Binds[0] = binds;
		bind.Binds[1] = binds >> 8;
		bind.BindingMode = Bindings[bindingNum].BindingMode;
		bind.Initialized = Bindings[bindingNum].Initialized;
		bind.Flags[0] = inverts[0];
		bind.Flags[1] = inverts[1];
		if (bind.DeviceNumber[0] > 32)
			bind.DeviceNumber[0] = 255;
		if (bind.DeviceNumber[1] > 32)
			bind.DeviceNumber[1] = 255;

		ValidateAndSetBinding(bindingNum, &bind);
	}
}

void test_controller_class::InitializeControls(bool useJoystick, bool useMouse)
{
	int i, j, k, stickNum;
	UseJoystick = useJoystick;
	UseMouse = useMouse;
	if (!useMouse)
	{
		MouseDX = 0;
		MouseDY = 0;
		MouseDZ = 0;
		MouseX = 0;
		MouseY = 0;
		MouseButtons = 0;
	}

	if (!useJoystick)
	{
		for (i = 0; i < NumDevices; i++)
		{
			stickNum = Controllers[i].StickNum;
			if (stickNum > -1)
			{
				JoystickStates[stickNum].x = Controllers[i].Ranges[0];
				JoystickStates[stickNum].y = Controllers[i].Ranges[1];
				JoystickStates[stickNum].z = Controllers[i].Ranges[2];
				JoystickStates[stickNum].r = Controllers[i].Ranges[3];
				JoystickStates[stickNum].u = Controllers[i].Ranges[4];
				JoystickStates[stickNum].v = Controllers[i].Ranges[5];

				for (j = 0; j < 8; j++)
				{
					JoystickStates[stickNum].HatRaw[j] = 255;
				}

				for (j = 0; j < 4; j++)
				{
					for (k = 0; k < 8; k++)
					{
						JoystickStates[stickNum].HatDownTime[j][k] = 0;
						JoystickStates[stickNum].HatReleaseTime[j][k] = 0;
						JoystickStates[stickNum].HatDownCount[j][k] = 0;
					}
				}

				for (j = 0; j < 32; j++)
				{
					JoystickStates[stickNum].ButtonState[j] = false;
					JoystickStates[stickNum].DownTime[j] = 0;
					JoystickStates[stickNum].UpTime[j] = 0;
				}
			}
		}
	}
}

int test_controller_class::GetMouseState(int* xPos, int* yPos) const
{
	if (ControlsSuspended) return 0;
	if (xPos) //[ISB] lint
		*xPos = MouseX;
	if (yPos)
		*yPos = MouseY;

	return MouseButtons;
}

int test_controller_class::GetJoyState(int* xPos, int* yPos) const
{
	int i;
	int stickNum, buttons;
	if (!ControlsSuspended)
	{
		for (i = 0; i < NumDevices; i++)
		{
			stickNum = Controllers[i].StickNum;

			if (xPos) //[ISB] lint
				*xPos = JoystickStates[i].x;
			if (yPos)
				*yPos = JoystickStates[i].y;
			buttons = JoystickStates[i].ButtonMask;

			if ((xPos && xPos != 0) || (yPos && yPos != 0) || buttons != 0)
				return buttons;
		}
	}

	return 0;
}

float test_controller_class::CheckAxisState(ubyte deviceNum, ubyte axisNum, int readMode, float param4)
{
	ControlState slideState, bankState;
	float value = 0;
	float scalar = 1.0;
	if (deviceNum < 0 || deviceNum >= NUM_CONTROLLERS)
		return 0;

	int stickNum = Controllers[deviceNum].StickNum;

	if (stickNum == -3 || !(Controllers[deviceNum].AxesMax & (1 << (axisNum - 1))))
		return 0;

	switch (axisNum - 1)
	{
	case 0:
		if (stickNum == -2)
			value = MouseDX;
		else
			value = JoystickStates[stickNum].x;
		break;
	case 1:
		if (stickNum == -2)
			value = MouseDY;
		else
			value = JoystickStates[stickNum].y;
		break;
	case 2:
		if (stickNum == -2)
			value = MouseDZ;
		else
			value = JoystickStates[stickNum].z;
		break;
	case 3:
		value = JoystickStates[stickNum].r;
		break;
	case 4:
		value = JoystickStates[stickNum].u;
		break;
	case 5:
		value = JoystickStates[stickNum].v;
		break;
	default: //I dunno
		value = param4;
		break;
	}
	if (stickNum == -2)
	{
		if (LastReadDelta < 0.005)
			LastReadDelta = 0.005;

		value *= LastReadDelta;
	}

	value /= Controllers[deviceNum].Ranges[axisNum - 1];

	value -= stickNum == -2 ? 0.0f : 1.0f;

	float deadzone = Controllers[deviceNum].Deadzone;
	if (deadzone < 0.05)
		deadzone = 0.05;

	if (value >= deadzone)
		value = (value - deadzone) / (1.0f - Controllers[deviceNum].Deadzone);
	else if (value <= -deadzone)
		value = (value + deadzone) / (1.0f - deadzone);
	else
		value = 0;

	value *= Controllers[deviceNum].Sensitivity[axisNum - 1] * Controllers[deviceNum].Unknown[axisNum - 1];

	value += 1.0;

	if (value < 0) value = 0;
	if (value > 2) value = 2;

	if (readMode == BMODE_DOWNSTATE)
	{
		if (value > 0.5)
			value = 1.0;
		else
			value = 0.0;
	}
	else if (readMode == BMODE_AXIS)
		value -= 1.0;
	else
		value = 0;

	ReadControlState(ctfTOGGLE_SLIDEKEY, &slideState, 0);
	ReadControlState(ctfTOGGLE_BANKKEY, &bankState, 0);

	if (slideState.TimeDown != 0 || bankState.TimeDown != 0)
		value = 0;

	//TODO: Mouselook

	return value;
}

float test_controller_class::CheckButtonState(ubyte deviceNum, int readMode, ubyte buttonNum)
{
	if (deviceNum < 0 || deviceNum >= NUM_CONTROLLERS)
		return 0;

	int stickNum = Controllers[deviceNum].StickNum;

	if (stickNum == -3 || Controllers[deviceNum].NumButtons < buttonNum)
		return 0;

	buttonNum--;
	if (readMode == BMODE_DOWNSTATE)
	{
		if (stickNum == -2)
		{
			if (MouseButtons & (1 << buttonNum))
				return 1.0f;
		}
		else
		{
			if (JoystickStates[stickNum].ButtonMask & 1 << (buttonNum & 31))
				return 1.0f;
		}
	}
	else if (readMode == BMODE_DOWNCOUNT)
	{
		if (stickNum == -2)
			return ddio_MouseBtnDownCount(buttonNum);

		ubyte count = JoystickStates[stickNum].ButtonState[buttonNum];
		JoystickStates[stickNum].ButtonState[buttonNum] = 0;
		return count;
	}
	else if (readMode == BMODE_DOWNTIME)
	{
		if (stickNum == -2)
			return ddio_MouseBtnDownTime(buttonNum);

		float time;
		if (JoystickStates[stickNum].ButtonMask & 1 << (buttonNum & 31))
		{
			time = ControlReadTime - JoystickStates[stickNum].DownTime[buttonNum];
			JoystickStates[stickNum].DownTime[buttonNum] = ControlReadTime;
		}
		else
		{
			time = ControlReadTime - JoystickStates[stickNum].UpTime[buttonNum];
			JoystickStates[stickNum].DownTime[buttonNum] = 0.0f;
		}
		JoystickStates[stickNum].UpTime[buttonNum] = 0.0f;
		return time;
	}

	return 0.0f;
}

float test_controller_class::CheckHatState(ubyte deviceNum, int readType, int hatNum, int position)
{
	float time;

	if (deviceNum < 0 || deviceNum >= NUM_CONTROLLERS)
		return 0;

	int stickNum = Controllers[deviceNum].StickNum;
	if (stickNum == -3)
		return 0;

	hatNum = hatNum & 255;
	//[ISB] \/ lint
	if (hatNum >= 4 || (64 << (hatNum & 31) & Controllers[deviceNum].AxesMax) == 0)
		return 0;

	int hatTest = (position & 255) >> 5;
	uint hatRaw = JoystickStates[stickNum].HatRaw[hatNum];
	uint hatBit = (hatRaw + (hatRaw >> 31 & 31)) >> 5;

	if (readType == BMODE_DOWNSTATE)
	{
		if (hatRaw == 255)
			return 0;
		if (position == 0 && (hatBit == 7 || hatBit == 0 || hatBit == 1))
			return 1;
		else if (position == 64 && (hatBit == 1 || hatBit == 2 || hatBit == 3))
			return 1;
		else if (position == 128 && (hatBit == 3 || hatBit == 4 || hatBit == 5))
			return 1;
		else if (position == 192 && (hatBit == 5 || hatBit == 6 || hatBit == 7))
			return 1;

		return 0;
	}
	else if (readType == BMODE_DOWNCOUNT)
	{
		if (hatTest != 8) //This shouldn't happen, since only 0-7 will come out the range 0-255
		{
			ubyte downCount = JoystickStates[stickNum].HatDownCount[hatNum][hatTest];
			JoystickStates[stickNum].HatDownCount[hatNum][hatTest] = 0;
			return downCount;
		}
	}
	else if (readType == BMODE_DOWNTIME)
	{
		if (hatBit == hatTest)
		{
			time = ControlReadTime - JoystickStates[stickNum].HatDownTime[hatNum][hatTest];
			JoystickStates[stickNum].HatDownTime[hatNum][hatTest] = ControlReadTime;
		}
		else
		{
			time = JoystickStates[stickNum].HatReleaseTime[hatNum][hatTest];
			JoystickStates[stickNum].HatDownTime[hatNum][hatTest] = 0;
		}
		JoystickStates[stickNum].HatReleaseTime[hatNum][hatTest] = 0;
		return time;
	}

	return 0;
}

float test_controller_class::CheckKeyState(int keynum, int readtype) const
{
	if (readtype == 2)
	{
		if (DDIO_key_state[keynum])
			return 1.0f;
	}
	else if (readtype == 3)
		return ddio_KeyDownCount(keynum);
	else if (readtype == 4)
		return ddio_KeyDownTime(keynum);

	return 0.0f;
}

int test_controller_class::BindFromDefaultEntry(ControlMatrixEntry* entry)
{
	int i;
	int hatnum;
	ControlBinding binding;

	for (i = 0; i < 2; i++)
	{
		binding.DeviceNumber[i] = 255;
		switch (entry->BindingType[i])
		{
		case CONTROL_AXIS:
			binding.DeviceNumber[i] = CheckAxis(entry->Binds[i]);
			break;
		case CONTROL_HAT:
		case CONTROL_JOYHAT2:
		case CONTROL_JOYHAT3:
		case CONTROL_JOYHAT4:
			if (entry->BindingType[i] == CONTROL_HAT)
				hatnum = 0;
			else
				hatnum = entry->BindingType[i] - 6;

			binding.DeviceNumber[i] = CheckControllerHat(hatnum);
			break;
		case CONTROL_JOYBUTTON:
			binding.DeviceNumber[i] = CheckControllerButton(entry->Binds[i]);
			break;
		case CONTROL_KEYBOARD:
			binding.DeviceNumber[i] = CONTROLLER_KEYBOARD;
			break;
		case CONTROL_MOUSEAXIS:
			binding.DeviceNumber[i] = CONTROLLER_MOUSE;
			break;
		case CONTROL_MOUSEBUTTON:
			if (Controllers[CONTROLLER_MOUSE].ButtonFlags & (1 << ((entry->Binds[i] - 1) & 31)))
				binding.DeviceNumber[i] = CONTROLLER_MOUSE;
			break;
		}

		binding.Binds[i] = entry->Binds[i];
	}

	binding.BindingMode = entry->BindingMode;
	binding.Flags[0] = binding.Flags[0];
	binding.Flags[1] = binding.Flags[1];
	binding.Initialized = true;
	ValidateAndSetBinding(entry->Control, &binding);
	return entry->Control;
}

void test_controller_class::ValidateAndSetBinding(int controlNum, ControlBinding* entry)
{
	int i;

	Bindings[controlNum].BindingMode = entry->BindingMode;
	Bindings[controlNum].Flags[0] = entry->Flags[0];
	Bindings[controlNum].Flags[1] = entry->Flags[1];
	Bindings[controlNum].Initialized = entry->Initialized;

	for (i = 0; i < 2; i++)
	{
		Bindings[controlNum].DeviceNumber[i] = entry->DeviceNumber[i];
		Bindings[controlNum].Binds[i] = entry->Binds[i];
		Bindings[controlNum].Types[i] = entry->Types[i];

		if (Bindings[controlNum].DeviceNumber[i] == 255)
			Bindings[controlNum].Binds[i] = 0;
		else
		{
			switch (entry->Types[i])
			{
			case CONTROL_AXIS:
			case CONTROL_HAT:
			case CONTROL_KEYBOARD:
			case CONTROL_MOUSEAXIS:
			case CONTROL_JOYHAT2:
			case CONTROL_JOYHAT3:
			case CONTROL_JOYHAT4:
				break;
			case CONTROL_JOYBUTTON:
			case CONTROL_MOUSEBUTTON:
				if (entry->Binds[i] > 32)
				{
					Bindings[controlNum].DeviceNumber[i] = 255;
					Bindings[controlNum].Binds[i] = 0;
				}
				break;
			default:
				Bindings[controlNum].Binds[i] = 0;
				Bindings[controlNum].DeviceNumber[i] = 255;
				break;
			}
		}
	}
}

ubyte test_controller_class::CheckAxis(int axisNum) const
{
	int i;
	for (i = NUM_FIXED_CONTROLLERS; i < NumDevices; i++)
	{
		if (Controllers[i].StickNum != -3 && Controllers[i].AxesMax & (1 << (axisNum - 1 & 31)))
			return i;
	}
	return 255;
}


ubyte test_controller_class::CheckControllerButton(int buttonNum) const
{
	int i;
	for (i = NUM_FIXED_CONTROLLERS; i < NumDevices; i++)
	{
		if (Controllers[i].StickNum != -3 && buttonNum < Controllers[i].NumButtons)
			return i;
	}
	return 255;
}

ubyte test_controller_class::CheckControllerHat(int axisNum) const
{
	int i;
	for (i = NUM_FIXED_CONTROLLERS; i < NumDevices; i++)
	{
		if (Controllers[i].StickNum != -3 && Controllers[i].AxesMax & (64 << (axisNum & 31)))
			return i;
	}
	return 255;
}

int test_controller_class::Init()
{
	int i;
	int left, top, right, bottom, znear, zfar;
	int buttonFlags, buttonCount, axes;

	for (i = 0; i < NUM_CONTROLLERS; i++)
		Controllers[i].StickNum = -3;

	Controllers[CONTROLLER_KEYBOARD].StickNum = -1;
	Controllers[CONTROLLER_KEYBOARD].NumButtons = 0;
	Controllers[CONTROLLER_KEYBOARD].AxesMax = 0;
	ddio_MouseGetLimits(&left, &top, &right, &bottom, &znear, &zfar);
	buttonFlags = ddio_MouseGetCaps(&buttonCount, &axes);
	Controllers[CONTROLLER_MOUSE].StickNum = -2;
	Controllers[CONTROLLER_MOUSE].NumButtons = buttonCount;
	Controllers[CONTROLLER_MOUSE].AxesMax = 3;
	Controllers[CONTROLLER_MOUSE].ButtonFlags = buttonFlags;
	Controllers[CONTROLLER_MOUSE].Ranges[0] = 320.0f;
	Controllers[CONTROLLER_MOUSE].Ranges[1] = 240.0f;
	Controllers[CONTROLLER_MOUSE].Ranges[2] = 100.0f;
	Controllers[CONTROLLER_MOUSE].Sensitivity[0] = 1.0f;
	Controllers[CONTROLLER_MOUSE].Sensitivity[1] = 1.0f;
	Controllers[CONTROLLER_MOUSE].Sensitivity[2] = 1.0f;
	Controllers[CONTROLLER_MOUSE].Unknown[0] = 1.0f;
	Controllers[CONTROLLER_MOUSE].Unknown[1] = 1.0f;
	Controllers[CONTROLLER_MOUSE].Unknown[2] = 1.0f;

	int numDevices = NUM_FIXED_CONTROLLERS;

	NumDevices = numDevices;
	ResetControls();
	return 1;
}

void test_controller_class::ReadJoyState(int stickNum)
{
}

void test_controller_class::ReadMouseState()
{
	int buttons, x, y, dX, dY;
	if (UseMouse && ControlFrameTime == 0.0f)
	{
		buttons = ddio_MouseGetState(&x, &y, &dX, &dY);
		MouseDX = dX;
		MouseDY = dY;
		MouseDZ = 0;
		MouseX = x;
		MouseY = y;
		MouseButtons = buttons;
	}
}

float test_controller_class::GetSensitivity(int controllerNum, ubyte axisNum) const
{
	if (controllerNum == 1)
		return Controllers[NUM_FIXED_CONTROLLERS].Sensitivity[axisNum - 1];
	else if (controllerNum == 5)
		return Controllers[CONTROLLER_MOUSE].Sensitivity[axisNum - 1];

	return 0;
}

void test_controller_class::SetSensitivity(int controllerNum, ubyte axisNum, float sensitivity)
{
	int i;
	if (controllerNum == 1)
	{
		for (i = NUM_FIXED_CONTROLLERS; i < NUM_CONTROLLERS; i++)
			Controllers[i].Sensitivity[axisNum - 1] = sensitivity;
	}
	else if (controllerNum == 5)
		Controllers[CONTROLLER_MOUSE].Sensitivity[axisNum - 1] = sensitivity;
}

void test_controller_class::SetDeadzone(int controllerNum, float deadzone)
{
	if (deadzone < 0)
		deadzone = 0.0f;
	else if (deadzone > 0.9f)
		deadzone = 0.9f;
	Controllers[controllerNum + NUM_FIXED_CONTROLLERS].Deadzone = deadzone;
}

const char EmptyHATString[16] = "";

const char JoyAxisLabels[][16] =
{
	"",
	"X-axis",
	"Y-axis",
	"Z-axis",
	"R-axis",
	"U-axis",
	"V-axis"
};

const char JoyButtonNames[][16] =
{
	"",
	"btn1",
	"btn2",
	"btn3",
	"btn4",
	"btn5",
	"btn6",
	"btn7",
	"btn8",
	"btn9",
	"btn10",
	"btn11",
	"btn12",
	"btn13",
	"btn14",
	"btn15",
	"btn16",
	"btn17",
	"btn18",
	"btn19",
	"btn20",
	"btn21",
	"btn22",
	"btn23",
	"btn24",
	"btn25",
	"btn26",
	"btn27",
	"btn28",
	"btn29",
	"btn30",
	"btn31",
	"btn32",
};

const char* test_controller_class::GetInputName(int controlType, ubyte axisNum, ubyte controlNum) const
{
	const char* buf;
	if (axisNum == 255)
		return nullptr;

	switch (controlType)
	{
	case CONTROL_AXIS:
		buf = JoyAxisLabels[controlNum];
		if (axisNum != 2 && (axisNum - 2) > -1)
		{
			snprintf(ControlNameBuffer, sizeof(ControlNameBuffer) - 1, "J%d:%s", axisNum - 1, buf);
			return ControlNameBuffer;
		}
		break;
	case CONTROL_HAT:
	case CONTROL_JOYHAT2:
	case CONTROL_JOYHAT3:
	case CONTROL_JOYHAT4:
		if (controlNum == 0)
			buf = "pov-U";
		else if (controlNum == 64)
			buf = "pov-R";
		else if (controlNum == 128)
			buf = "pov-D";
		else if (controlNum == 192)
			buf = "pov-L";
		else
			buf = EmptyHATString;

		if (axisNum != 2 && (axisNum - 2) > -1)
		{
			if (controlType != CONTROL_HAT)
			{
				snprintf(ControlNameBuffer, sizeof(ControlNameBuffer) - 1, "J%d:%s%d", axisNum - 1, buf, controlType - 6);
				return ControlNameBuffer;
			}
			snprintf(ControlNameBuffer, sizeof(ControlNameBuffer) - 1, "J%d:%s", axisNum - 1, buf);
			return ControlNameBuffer;
		}
		else if (controlNum != CONTROL_HAT)
		{
			snprintf(ControlNameBuffer, sizeof(ControlNameBuffer) - 1, "%s%d", buf, axisNum - 1);
			return ControlNameBuffer;
		}
		break;
	case CONTROL_JOYBUTTON:
		buf = JoyButtonNames[controlNum];
		if (axisNum != 2 && (axisNum - 2) > -1)
		{
			snprintf(ControlNameBuffer, sizeof(ControlNameBuffer) - 1, "J%d:%s", axisNum - 1, buf);
			return ControlNameBuffer;
		}
		break;
	case CONTROL_KEYBOARD:
		buf = ControlNameBuffer;
		break;
	case CONTROL_MOUSEAXIS:
		return ddio_MouseGetAxisText(controlNum - 1);
	case CONTROL_MOUSEBUTTON:
		return ddio_MouseGetBtnText(controlNum - 1);
	default:
		ControlNameBuffer[0] = '\0';
		buf = ControlNameBuffer;
		break;
	}

	return buf;
}

controller_class* CreateController(int numcontrols, ControlMatrixEntry* defaultcontrols, char* ip)
{
	controller_class* controller = nullptr;

	//TODO: check that the exception handler is actually correct
	controller = new test_controller_class(numcontrols, defaultcontrols);

	return controller;
}

void DestroyController(controller_class* controller)
{
	if (controller)
		delete controller;
}*/

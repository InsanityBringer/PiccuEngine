#include "application.h"

#include <stdlib.h>
#include <math.h>
#include <stdexcept>

#include "descent.h"
#include "controls.h"
#include "Controller.h"
#include "ddio.h"
#include "ddio_common.h"

//Sorry! This is needed for the semi-hacky mouselook support
#include "descent.h"
#include "player.h"
#include "object.h"
#include "pilot.h"
#include "multi.h"
#include "game.h"
//End of hacky includes

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

#define NUM_AXISBINDSTRINGS		(sizeof(Ctltext_AxisBindings)/sizeof(Ctltext_AxisBindings[0]))
#define NUM_BTNBINDSTRINGS			(sizeof(Ctltext_BtnBindings)/sizeof(Ctltext_AxisBindings[0]))

static float WinControllerTimer = 0.0f;
static longlong g_last_frame_timer_ms = -1;
static float g_accum_frame_time = 0.0f;

#define JOY_DEADZONE	0.20f
#define MOUSE_DEADZONE 0.00f

gameWinController::gameWinController(int num_funcs, ct_function* funcs, char* remote_adr)
	: gameController(num_funcs, funcs)
{
	enum_controllers(remote_adr);

	for (int i = 0; i < num_funcs; i++)
		assign_function(&funcs[i]);

	m_Suspended = 0;
	m_frame_timer_ms = -1;
	m_frame_time = 1.0f;
	g_last_frame_timer_ms = -1;
	g_accum_frame_time = 0.0f;

	gameWinController::flush();
}

gameWinController::~gameWinController()
{
}

void gameWinController::suspend()
{
	m_Suspended = 1;
}

void gameWinController::resume()
{
	m_Suspended = 0; m_frame_timer_ms = -1; m_frame_time = 1.0f;
}

#define CONTROLLER_POLLING_TIME	50
#define MOUSE_POLLING_TIME	(1.0f/20.0f)

void gameWinController::poll()
{
	longlong cur_frame_timer_ms;

	if (m_Suspended)
		return;

	cur_frame_timer_ms = timer_GetMSTime();
	if (m_frame_timer_ms == -1) 
	{
		// don't poll this frame.
		m_frame_timer_ms = cur_frame_timer_ms;
		g_last_frame_timer_ms = cur_frame_timer_ms;
		g_accum_frame_time = 0.0f;
		return;
	}

	m_frame_time = (float)((cur_frame_timer_ms - m_frame_timer_ms) / 1000.0);
	m_frame_timer_ms = cur_frame_timer_ms;
	g_accum_frame_time += m_frame_time;

	if (g_accum_frame_time >= MOUSE_POLLING_TIME) 
	{
		g_accum_frame_time = 0.0f;
	}

	for (int ctl = 0; ctl < m_NumControls; ctl++)
	{
		if (m_ControlList[ctl].id >= CTID_EXTCONTROL0) {
			extctl_getpos(m_ControlList[ctl].id);
		}
		else if (m_ControlList[ctl].id == CTID_MOUSE) {
			//	if ((cur_frame_timer_ms - g_last_frame_timer_ms) > CONTROLLER_POLLING_TIME) {
			mouse_geteval();
			//		g_last_frame_timer_ms = cur_frame_timer_ms;
			//	}
		}
	}
}

void gameWinController::flush()
{
	bool old_mse = m_MouseActive, old_joy = m_JoyActive;

	ddio_KeyFlush();
	ddio_MouseQueueFlush();

	// does real flush
	mask_controllers(false, false);
	mask_controllers(old_joy, old_mse);
}

ct_config_data gameWinController::get_controller_value(ct_type type_req)
{
	//	will return the current value of a requested control type.
	ct_config_data val = MAKE_CONFIG_DATA(INVALID_CONTROLLER_INFO, NULL_BINDING);
	int i, j;

	switch (type_req)
	{
		int pov_n;

	case ctKey:
		val = makeword(0, ddio_KeyInKey());
		break;

	case ctButton:
		for (i = 2; i < m_NumControls; i++)
		{
			for (j = 0; j < m_ControlList[i].buttons; j++)
			{
				if (m_ExtCtlStates[m_ControlList[i].id].btnpresses[j] && !(m_ExtCtlStates[m_ControlList[i].id].buttons & (1 << j))) {
					val = MAKE_CONFIG_DATA(CONTROLLER_CTL_INFO(i, NULL_CONTROLLER), CONTROLLER_CTL_VALUE(j + 1, NULL_BINDING));
					return val;
				}
			}
		}
		break;

	case ctMouseButton:
		for (j = 0; j < CT_MAX_BUTTONS; j++)
		{
			if (ddio_MouseBtnUpCount(j)) {
				//	mprintf((0, "MseBtn %d down\n", j));
				val = MAKE_CONFIG_DATA(CONTROLLER_CTL_INFO(1, NULL_CONTROLLER), CONTROLLER_CTL_VALUE(j + 1, NULL_BINDING));
				return val;
			}
		}
		break;

	case ctAxis:
		for (i = 2; i < m_NumControls; i++)
		{
			float pos;
			float limit;
			unsigned ctl = CONTROLLER_CTL_INFO(i, NULL_CONTROLLER);

			if (m_ControlList[i].flags & CTF_V_AXIS) {
				limit = (m_ControlList[i].sens[CT_V_AXIS - 1] > 1.5f) ? 0.95f : (m_ControlList[i].sens[CT_V_AXIS - 1] > 1.0f) ? 0.80f : (m_ControlList[i].sens[CT_V_AXIS - 1] / 2);
				pos = get_axis_value(i, CT_V_AXIS, ctAnalog);
				if (fabs(pos) > limit)
					val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_V_AXIS, NULL_BINDING));
			}
			if (m_ControlList[i].flags & CTF_U_AXIS) {
				limit = (m_ControlList[i].sens[CT_U_AXIS - 1] > 1.5f) ? 0.95f : (m_ControlList[i].sens[CT_U_AXIS - 1] > 1.0f) ? 0.80f : (m_ControlList[i].sens[CT_U_AXIS - 1] / 2);
				pos = get_axis_value(i, CT_U_AXIS, ctAnalog);
				if (fabs(pos) > limit)
					val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_U_AXIS, NULL_BINDING));
			}
			if (m_ControlList[i].flags & CTF_R_AXIS) {
				limit = (m_ControlList[i].sens[CT_R_AXIS - 1] > 1.5f) ? 0.95f : (m_ControlList[i].sens[CT_R_AXIS - 1] > 1.0f) ? 0.80f : (m_ControlList[i].sens[CT_R_AXIS - 1] / 2);
				pos = get_axis_value(i, CT_R_AXIS, ctAnalog);
				if (fabs(pos) > limit)
					val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_R_AXIS, NULL_BINDING));
			}
			if (m_ControlList[i].flags & CTF_Z_AXIS) {
				limit = (m_ControlList[i].sens[CT_Z_AXIS - 1] > 1.5f) ? 0.95f : (m_ControlList[i].sens[CT_Z_AXIS - 1] > 1.0f) ? 0.80f : (m_ControlList[i].sens[CT_Z_AXIS - 1] / 2);
				pos = get_axis_value(i, CT_Z_AXIS, ctAnalog);
				if (fabs(pos) > limit)
					val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_Z_AXIS, NULL_BINDING));
			}
			if (m_ControlList[i].flags & CTF_Y_AXIS) {
				limit = (m_ControlList[i].sens[CT_Y_AXIS - 1] > 1.5f) ? 0.95f : (m_ControlList[i].sens[CT_Y_AXIS - 1] > 1.0f) ? 0.80f : (m_ControlList[i].sens[CT_Y_AXIS - 1] / 2);
				pos = get_axis_value(i, CT_Y_AXIS, ctAnalog);
				if (fabs(pos) > limit)
					val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_Y_AXIS, NULL_BINDING));
			}
			if (m_ControlList[i].flags & CTF_X_AXIS) {
				limit = (m_ControlList[i].sens[CT_X_AXIS - 1] > 1.5f) ? 0.95f : (m_ControlList[i].sens[CT_X_AXIS - 1] > 1.0f) ? 0.80f : (m_ControlList[i].sens[CT_X_AXIS - 1] / 2);
				pos = get_axis_value(i, CT_X_AXIS, ctAnalog);
				if (fabs(pos) > limit)
					val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_X_AXIS, NULL_BINDING));
			}
		}
		break;

	case ctMouseAxis:
	{
		float pos = 0.0f;
		unsigned ctl = CONTROLLER_CTL_INFO(1, NULL_CONTROLLER), i = 1;

		ASSERT(m_ControlList[i].id == CTID_MOUSE);

		if (m_ControlList[i].flags & CTF_V_AXIS) {
			pos = get_axis_value(i, CT_V_AXIS, ctAnalog);
			if (fabs(pos) >= 0.50f)
				val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_V_AXIS, NULL_BINDING));
		}
		if (m_ControlList[i].flags & CTF_U_AXIS) {
			pos = get_axis_value(i, CT_U_AXIS, ctAnalog);
			if (fabs(pos) >= 0.50f)
				val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_U_AXIS, NULL_BINDING));
		}
		if (m_ControlList[i].flags & CTF_R_AXIS) {
			pos = get_axis_value(i, CT_R_AXIS, ctAnalog);
			if (fabs(pos) >= 0.90f)
				val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_R_AXIS, NULL_BINDING));
		}
		if (m_ControlList[i].flags & CTF_Z_AXIS) {
			pos = get_axis_value(i, CT_Z_AXIS, ctAnalog);
			if (fabs(pos) >= 0.50f)
				val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_Z_AXIS, NULL_BINDING));
		}
		if (m_ControlList[i].flags & CTF_Y_AXIS) {
			pos = get_axis_value(i, CT_Y_AXIS, ctAnalog);
			//	mprintf((0, "y=%.2f   ", pos));
			if (fabs(pos) >= 0.90f)
				val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_Y_AXIS, NULL_BINDING));
		}
		if (m_ControlList[i].flags & CTF_X_AXIS) {
			pos = get_axis_value(i, CT_X_AXIS, ctAnalog);
			//	mprintf((0, "x=%.2f\n", pos));
			if (fabs(pos) >= 0.90f)
				val = MAKE_CONFIG_DATA(ctl, CONTROLLER_CTL_VALUE(CT_X_AXIS, NULL_BINDING));
		}
	}
	break;

	case ctPOV:
	case ctPOV2:
	case ctPOV3:
	case ctPOV4:
		if (type_req == ctPOV) {
			pov_n = 0;
		}
		else {
			pov_n = (type_req - ctPOV2) + 1;
		}
		for (i = 2; i < m_NumControls; i++)
		{
			float pos;
			int ctl = CONTROLLER_CTL_INFO(i, -1);

			if (m_ControlList[i].flags & (CTF_POV << pov_n)) {
				pos = get_pov_value(i, ctDigital, pov_n, JOYPOV_RIGHT);
				if (pos) val = makeword(ctl, CONTROLLER_CTL_VALUE(JOYPOV_RIGHT, 0));
			}
			if (m_ControlList[i].flags & (CTF_POV << pov_n)) {
				pos = get_pov_value(i, ctDigital, pov_n, JOYPOV_LEFT);
				if (pos) val = makeword(ctl, CONTROLLER_CTL_VALUE(JOYPOV_LEFT, 0));
			}
			if (m_ControlList[i].flags & (CTF_POV << pov_n)) {
				pos = get_pov_value(i, ctDigital, pov_n, JOYPOV_DOWN);
				if (pos) val = makeword(ctl, CONTROLLER_CTL_VALUE(JOYPOV_DOWN, 0));
			}
			if (m_ControlList[i].flags & (CTF_POV << pov_n)) {
				pos = get_pov_value(i, ctDigital, pov_n, JOYPOV_UP);
				if (pos) val = makeword(ctl, CONTROLLER_CTL_VALUE(JOYPOV_UP, 0));
			}
		}
		break;
	}

	return val;
}

void gameWinController::set_controller_function(int id, const ct_type* type, ct_config_data value, const ubyte* flags)
{
	//[ISB] lint
	if (!type || !flags)
	{
		Int3();
		return;
	}

	ct_element elem;

	if (id >= CT_MAX_ELEMENTS) return;

	// auto assign keyboard controller if type is key.
	if (type[0] == ctKey)
		elem.ctl[0] = CONTROLLER_CTL1_INFO(0);
	else
		elem.ctl[0] = CONTROLLER_CTL1_INFO(CONTROLLER_INFO(value));

	if (type[1] == ctKey)
		elem.ctl[1] = CONTROLLER_CTL2_INFO(0);
	else
		elem.ctl[1] = CONTROLLER_CTL2_INFO(CONTROLLER_INFO(value));

	elem.ctype[0] = type[0];
	elem.ctype[1] = type[1];
	elem.format = m_ElementList[id].format;
	elem.value[0] = CONTROLLER_CTL1_VALUE(CONTROLLER_VALUE(value));
	elem.value[1] = CONTROLLER_CTL2_VALUE(CONTROLLER_VALUE(value));
	elem.flags[0] = flags[0];
	elem.flags[1] = flags[1];
	elem.enabled = m_ElementList[id].enabled;

	//	if controller doesn't exist, set it to invalid.
	if (elem.ctl[0] > CT_MAX_CONTROLLERS)
		elem.ctl[0] = NULL_WINCONTROLLER;
	if (elem.ctl[1] >= CT_MAX_CONTROLLERS)
		elem.ctl[1] = NULL_WINCONTROLLER;

	assign_element(id, &elem);
}

void gameWinController::get_controller_function(int id, ct_type* type, ct_config_data* value, ubyte* flags)
{
	//[ISB] lint
	if (!type || !flags || !value)
	{
		Int3();
		return;
	}

	type[0] = m_ElementList[id].ctype[0];
	type[1] = m_ElementList[id].ctype[1];
	*value = makeword(CONTROLLER_CTL_INFO(m_ElementList[id].ctl[0], m_ElementList[id].ctl[1]),
		CONTROLLER_CTL_VALUE(m_ElementList[id].value[0], m_ElementList[id].value[1]));
	flags[0] = m_ElementList[id].flags[0];
	flags[1] = m_ElementList[id].flags[1];
}

void gameWinController::enable_function(int id, bool enable)
{
	m_ElementList[id].enabled = enable;
}

bool gameWinController::get_packet(int id, ct_packet* packet, ct_format alt_format)
{
	float val = (float)0.0;
	int i;

	ASSERT(id < CT_MAX_ELEMENTS);

	packet->format = (alt_format != ctNoFormat) ? alt_format : m_ElementList[id].format;
	alt_format = packet->format;

	WinControllerTimer = timer_GetTime();
	packet->flags = 0;

	if (!m_ElementList[id].enabled) {
		goto skip_packet_read;
	}

	//	check if the element's controller is valid.

	for (i = 0; i < CTLBINDS_PER_FUNC; i++)
	{
		ubyte value = m_ElementList[id].value[i];
		sbyte controller = m_ElementList[id].ctl[i];

		if (controller == -1 || m_ControlList[controller].id == CTID_INVALID) {
			continue;
		}
		switch (m_ElementList[id].ctype[i])
		{
		case ctKey:
			if (value) {
				val = get_key_value(value, alt_format);
				if (KEY_STATE(value)) packet->flags |= CTPK_ELEMENTACTIVE;
			}
			break;

		case ctMouseAxis:
			packet->flags |= CTPK_MOUSE;
		case ctAxis:
			val = get_axis_value(controller, value, alt_format, (m_ElementList[id].flags[i] & CTFNF_INVERT) ? true : false);
			if (m_ElementList[id].flags[i] & CTFNF_INVERT) {
				if (alt_format == ctDigital) {
					val = (val == 0.0f) ? 1.0f : 0.0f;
				}
				else if (alt_format == ctAnalog) {
					val = -val;
				}
			}
			break;

		case ctMouseButton:
			packet->flags |= CTPK_MOUSE;
		case ctButton:
			val = get_button_value(controller, alt_format, value);
			break;

		case ctPOV:
			val = get_pov_value(controller, alt_format, 0, value);
			break;
		case ctPOV2:
		case ctPOV3:
		case ctPOV4:
			val = get_pov_value(controller, alt_format, (m_ElementList[id].ctype[i] - ctPOV2) + 1, value);
			break;

		default:
			Int3();
			val = 0.0f;
		}

		if (val)
			break;
	}

skip_packet_read:
	if (val) packet->flags |= CTPK_ELEMENTACTIVE;

	packet->value = val;

	return true;
}

float gameWinController::get_axis_sensitivity(ct_type axis_type, ubyte axis)
{
	axis--;
	ASSERT(axis < CT_NUM_AXES);

	switch (axis_type)
	{
	case ctMouseAxis:
		return m_ControlList[1].sens[axis];

	case ctAxis:
		return m_ControlList[2].sens[axis];

	default:
		Int3();
	}

	return 0.0f;
}

void gameWinController::set_axis_sensitivity(ct_type axis_type, ubyte axis, float val)
{
	int i;

	axis--;
	ASSERT(axis < CT_NUM_AXES);

	switch (axis_type)
	{
	case ctMouseAxis:
		m_ControlList[1].sens[axis] = val;
		break;
	case ctAxis:
		for (i = 2; i < CT_MAX_CONTROLLERS; i++)
			m_ControlList[i].sens[axis] = val;
		break;
	default:
		Int3();
	}
}

int gameWinController::assign_function(ct_function* fn)
{
	//	for now this is a straight forward translation (that is, no mapping of needs to controller
	//	list to create elements.
	ct_element elem;
	int i;

	for (i = 0; i < CTLBINDS_PER_FUNC; i++)
	{
		elem.ctl[i] = NULL_WINCONTROLLER;

		switch (fn->ctype[i])
		{
			int pov_n;
		case ctNone:
			break;
		case ctKey:
			elem.ctl[i] = 0;
			break;
		case ctAxis:
			elem.ctl[i] = get_axis_controller(fn->value[i]);
			break;

		case ctButton:
			elem.ctl[i] = get_button_controller(fn->value[i]);
			break;

		case ctMouseAxis:
			elem.ctl[i] = 1;
			break;

		case ctMouseButton:
			//	find a free mouse button.
			if ((m_ControlList[1].btnmask & (1 << (fn->value[i] - 1))) && ((fn->value[i] - 1) < m_ControlList[1].buttons)) {
				elem.ctl[i] = 1;
			}
			break;

		case ctPOV:
		case ctPOV2:
		case ctPOV3:
		case ctPOV4:
			if (fn->ctype[i] == ctPOV)
				pov_n = 0;
			else
				pov_n = (fn->ctype[i] - ctPOV2) + 1;

			elem.ctl[i] = get_pov_controller(pov_n);
			break;
		}

		elem.ctype[i] = fn->ctype[i];
		elem.value[i] = fn->value[i];
	}

	elem.format = fn->format;
	elem.flags[0] = fn->flags[0];
	elem.flags[1] = fn->flags[1];
	elem.enabled = true;

	assign_element(fn->id, &elem);

	return fn->id;
}

void gameWinController::mask_controllers(bool joystick, bool mouse)
{
	int i, j, k, stickNum;
	m_JoyActive = joystick;
	m_MouseActive = mouse;
	if (!mouse)
	{
		m_MseState.m_deltaX = 0;
		m_MseState.m_deltaY = 0;
		m_MseState.m_deltaZ = 0;
		m_MseState.m_absX = 0;
		m_MseState.m_absY = 0;
		m_MseState.m_buttonMask = 0;
	}

	if (!joystick)
	{
		for (i = 0; i < m_NumControls; i++)
		{
			stickNum = m_ControlList[i].id;
			if (stickNum >= 0)
			{
				m_ExtCtlStates[stickNum].x = m_ControlList[i].normalizer[0];
				m_ExtCtlStates[stickNum].y = m_ControlList[i].normalizer[1];
				m_ExtCtlStates[stickNum].z = m_ControlList[i].normalizer[2];
				m_ExtCtlStates[stickNum].r = m_ControlList[i].normalizer[3];
				m_ExtCtlStates[stickNum].u = m_ControlList[i].normalizer[4];
				m_ExtCtlStates[stickNum].v = m_ControlList[i].normalizer[5];

				for (j = 0; j < JOYPOV_NUM; j++)
				{
					m_ExtCtlStates[stickNum].pov[j] = 255;
				}

				for (j = 0; j < JOYPOV_NUM; j++)
				{
					for (k = 0; k < JOYPOV_DIR; k++)
					{
						m_ExtCtlStates[stickNum].povstarts[j][k] = 0;
						m_ExtCtlStates[stickNum].povtimes[j][k] = 0;
						m_ExtCtlStates[stickNum].povpresses[j][k] = 0;
					}
				}

				for (j = 0; j < CT_MAX_BUTTONS; j++)
				{
					m_ExtCtlStates[stickNum].btnpresses[j] = 0;
					m_ExtCtlStates[stickNum].btnstarts[j] = 0;
					m_ExtCtlStates[stickNum].btntimes[j] = 0;
				}
			}
		}
	}
}

const char* gameWinController::get_binding_text(ct_type type, ubyte ctrl, ubyte bind)
{
	static char binding_text[16];
	const char* str;

	if (ctrl == NULL_CONTROLLER) {
		return NULL;
	}

	switch (type)
	{
		int pov_n;
	case ctAxis:
	{
		ASSERT(bind < NUM_AXISBINDSTRINGS);
		str = Ctltext_AxisBindings[bind];
		if ((ctrl - 2) > 0) {
			sprintf(binding_text, "J%d:%s", (ctrl - 2) + 1, str);
		}
		else {
			return str;
		}
		break;
	}

	case ctMouseAxis:
	{
		str = ddio_MouseGetAxisText(((sbyte)bind) - 1);
		return str;
	}

	case ctButton:
	{
		ASSERT(bind < NUM_BTNBINDSTRINGS);
		str = Ctltext_BtnBindings[bind];
		if ((ctrl - 2) > 0) {
			sprintf(binding_text, "J%d:%s", (ctrl - 2) + 1, str);
		}
		else {
			return str;
		}
		break;
	}

	case ctMouseButton:
	{
		str = ddio_MouseGetBtnText(((sbyte)bind) - 1);
		return str;
	}

	case ctPOV:
	case ctPOV2:
	case ctPOV3:
	case ctPOV4:
	{
		ushort povpos = bind;

		if (type == ctPOV)
			pov_n = 0;
		else
			pov_n = (type - ctPOV2) + 1;

		if (povpos == JOYPOV_UP)
			str = Ctltext_PovBindings[1];
		else if (povpos == JOYPOV_DOWN)
			str = Ctltext_PovBindings[3];
		else if (povpos == JOYPOV_LEFT)
			str = Ctltext_PovBindings[4];
		else if (povpos == JOYPOV_RIGHT)
			str = Ctltext_PovBindings[2];
		else
			str = Ctltext_PovBindings[0];
		if ((ctrl - 2) > 0) {
			if (pov_n) {
				sprintf(binding_text, "J%d:%s%d", (ctrl - 2) + 1, str, pov_n);
			}
			else {
				sprintf(binding_text, "J%d:%s", (ctrl - 2) + 1, str);
			}
		}
		else {
			if (pov_n) {
				sprintf(binding_text, "%s%d", str, pov_n);
			}
			else {
				return str;
			}
		}
		break;
	}

	case ctKey:
		break;

	default:
		if (type == ctNone) {
			Int3();
		}
		binding_text[0] = 0;
	}

	return binding_text;
}

int gameWinController::get_mouse_raw_values(int* x, int* y)
{
	if (m_Suspended)
		return 0;

	*x = m_MseState.m_absX;
	*y = m_MseState.m_absY;

	return m_MseState.m_buttonMask;
}

unsigned gameWinController::get_joy_raw_values(int* x, int* y)
{
	unsigned btn = 0;

	if (m_Suspended)
		return 0;


	for (int ctl = 0; ctl < m_NumControls; ctl++)
	{
		int dev = m_ControlList[ctl].id;

		if (dev >= CTID_EXTCONTROL0) 
		{
			*x = m_ExtCtlStates[dev].x;
			*y = m_ExtCtlStates[dev].y;
			btn = m_ExtCtlStates[dev].buttons;
			if (*x || *y || btn)
				return btn;
		}
	}

	return 0;
}

void gameWinController::set_controller_deadzone(int ctl, float deadzone)
{
	if (ctl < 0 || ctl >= (m_NumControls - 2)) {
		return;
	}

	if (deadzone < 0.0f) deadzone = 0.0f;
	if (deadzone > 0.9f) deadzone = 0.9f;

	m_ControlList[ctl + 2].deadzone = deadzone;
}

bool gameWinController::enum_controllers(char* remote_adr)
{
	int i;
	int left, top, right, bottom, znear, zfar;
	int buttonFlags, buttonCount, axes;

	for (i = 0; i < CT_MAX_CONTROLLERS; i++)
		m_ControlList[i].id = CTID_INVALID;

	m_ControlList[CONTROLLER_KEYBOARD].id = CTID_KEYBOARD;
	m_ControlList[CONTROLLER_KEYBOARD].buttons = 0;
	m_ControlList[CONTROLLER_KEYBOARD].flags = 0;
	ddio_MouseGetLimits(&left, &top, &right, &bottom, &znear, &zfar);
	buttonFlags = ddio_MouseGetCaps(&buttonCount, &axes);
	m_ControlList[CONTROLLER_MOUSE].id = CTID_MOUSE;
	m_ControlList[CONTROLLER_MOUSE].buttons = buttonCount;
	m_ControlList[CONTROLLER_MOUSE].flags = 3;
	m_ControlList[CONTROLLER_MOUSE].btnmask = buttonFlags;
	m_ControlList[CONTROLLER_MOUSE].normalizer[0] = 320.0f;
	m_ControlList[CONTROLLER_MOUSE].normalizer[1] = 240.0f;
	m_ControlList[CONTROLLER_MOUSE].normalizer[2] = 100.0f;
	m_ControlList[CONTROLLER_MOUSE].sens[0] = 1.0f;
	m_ControlList[CONTROLLER_MOUSE].sens[1] = 1.0f;
	m_ControlList[CONTROLLER_MOUSE].sens[2] = 1.0f;
	m_ControlList[CONTROLLER_MOUSE].sensmod[0] = 1.0f;
	m_ControlList[CONTROLLER_MOUSE].sensmod[1] = 1.0f;
	m_ControlList[CONTROLLER_MOUSE].sensmod[2] = 1.0f;

	int numDevices = NUM_FIXED_CONTROLLERS;

	m_NumControls = numDevices;

	flush();
	return true;
}

void gameWinController::assign_element(int id, ct_element* elem)
{
	//	assign element, check to see if valid.
	int i;

	m_ElementList[id].format = elem->format;
	m_ElementList[id].flags[0] = elem->flags[0];
	m_ElementList[id].flags[1] = elem->flags[1];
	m_ElementList[id].enabled = elem->enabled;

	//	look through each controller and validate each element
	for (i = 0; i < CTLBINDS_PER_FUNC; i++)
	{
		m_ElementList[id].ctl[i] = elem->ctl[i];
		m_ElementList[id].value[i] = elem->value[i];
		m_ElementList[id].ctype[i] = elem->ctype[i];

		if (m_ElementList[id].ctl[i] != NULL_WINCONTROLLER) 
		{
			// this function shouldn't do any error checking!!!!  keep same controller values and bindings unless
			// bindings are truly bogus.
			switch (m_ElementList[id].ctype[i])
			{
			case ctMouseButton:
			case ctButton:
				if (elem->value[i] > CT_MAX_BUTTONS) 
				{
					m_ElementList[id].ctl[i] = NULL_WINCONTROLLER;
					m_ElementList[id].value[i] = NULL_BINDING;
				}
				break;
			case ctMouseAxis:
			case ctAxis:
				//[ISB] these are empty even in the Windows code from ghidra
				//					if (!(m_ControlList[elem->ctl[i]].flags & (1<<(elem->value[i]-1))))	
				//						m_ElementList[id].ctl[i] = NULL_WINCONTROLLER;
				break;
			case ctPOV:
			case ctPOV2:
			case ctPOV3:
			case ctPOV4:
				//					if (!(m_ControlList[elem->ctl[i]].flags & CTF_POV)) 
				//						m_ElementList[id].ctl[i] = NULL_WINCONTROLLER;
				break;
			case ctKey:
				break;
			default:
				m_ElementList[id].value[i] = NULL_BINDING;
				m_ElementList[id].ctl[i] = NULL_WINCONTROLLER;
			}
		}
		else {
			m_ElementList[id].value[i] = NULL_BINDING;
		}
	}
}

sbyte gameWinController::get_axis_controller(ubyte axis)
{
	//	start from controller 2 because 0, and 1 are reserved for keyboard and mouse
	if (axis == NULL_BINDING)
		return NULL_WINCONTROLLER;

	for (int i = 2; i < m_NumControls; i++)
		if ((m_ControlList[i].flags & (1 << (axis - 1))) && m_ControlList[i].id != CTID_INVALID) return i;

	return NULL_WINCONTROLLER;
}

sbyte gameWinController::get_button_controller(ubyte btn)
{
	unsigned mask;

	//	buttons range from 1-CT_MAX_BUTTONS
	ASSERT(btn <= CT_MAX_BUTTONS);
	if (btn == NULL_BINDING)
		return NULL_WINCONTROLLER;

	mask = 1 << (btn - 1);

	//	start from controller 2 because 0, and 1 are reserved for keyboard and mouse
	for (int i = 2; i < m_NumControls; i++)
		//@@		if (((unsigned)btn < m_ControlList[i].buttons) && !(m_ControlList[i].btnmask & mask) && (m_ControlList[i].id != CTID_INVALID)) {
		if (((unsigned)btn < m_ControlList[i].buttons) && (m_ControlList[i].id != CTID_INVALID)) {
			//@@			m_ControlList[i].btnmask |= mask;
			return i;
		}

	return NULL_WINCONTROLLER;
}

sbyte gameWinController::get_pov_controller(ubyte pov)
{
	//	start from controller 2 because 0, and 1 are reserved for keyboard and mouse
	ushort pov_flag = CTF_POV << (pov);

	for (int i = 2; i < m_NumControls; i++)
		if ((m_ControlList[i].flags & pov_flag) && m_ControlList[i].id != CTID_INVALID)
			return i;

	return NULL_WINCONTROLLER;
}

float gameWinController::get_axis_value(sbyte controller, ubyte axis, ct_format format, bool invert)
{
	struct gameWinController::t_controller* ctldev;
	float val = (float)0.0;
	float normalizer, axisval, nullzone;	//, senszone;

	if (controller < 0 || controller >= CT_MAX_CONTROLLERS) 
	{
		return 0.0f;
	}

	ctldev = &m_ControlList[controller];
	if (ctldev->id == CTID_INVALID) 
	{
		return 0.0f;
	}

#ifdef _DEBUG
	if (m_ControlList[controller].id == CTID_KEYBOARD) 
	{
		Int3();
		return 0.0f;
	}
#endif

	//	verify controller axis
	if (!CHECK_FLAG(ctldev->flags, 1 << (axis - 1))) {
		return val;
	}

	//	get raw value
	switch (axis)
	{
		//	note we take care of mouse controls and external controls here
	case CT_X_AXIS: axisval = (float)((ctldev->id == CTID_MOUSE) ? m_MseState.m_deltaX : m_ExtCtlStates[ctldev->id].x); break;
	case CT_Y_AXIS: axisval = (float)((ctldev->id == CTID_MOUSE) ? m_MseState.m_deltaY : m_ExtCtlStates[ctldev->id].y); break;
	case CT_Z_AXIS: axisval = (float)((ctldev->id == CTID_MOUSE) ? m_MseState.m_deltaZ : m_ExtCtlStates[ctldev->id].z); break;
	case CT_R_AXIS: axisval = (float)m_ExtCtlStates[ctldev->id].r; break;
	case CT_U_AXIS: axisval = (float)m_ExtCtlStates[ctldev->id].u; break;
	case CT_V_AXIS: axisval = (float)m_ExtCtlStates[ctldev->id].v; break;
	default: Int3();				// NOT A VALID AXIS
	}

	// create normalizer
	axis--;
	if (ctldev->id == CTID_MOUSE) {
		if (m_frame_time < 0.005f) m_frame_time = 0.005f;			// to trap potential errors.
		normalizer = ctldev->normalizer[axis] * m_frame_time;
		nullzone = MOUSE_DEADZONE;
		if (axis == CT_X_AXIS) {
			//mprintf_at((4, 4, 0, "m_dX:%03d  normal:%03.2f", (int)axisval, normalizer));
		}

	}
	else {
		normalizer = ctldev->normalizer[axis];
		nullzone = (m_ControlList[controller].deadzone < 0.05f) ? 0.05f : m_ControlList[controller].deadzone;
	}

	val = axisval / normalizer;
	val = val - ((ctldev->id == CTID_MOUSE) ? 0.0f : 1.0f);		// joystick needs to be normalized to -1.0 to 1.0

	//	calculate adjusted value
	if (val > nullzone) {
		val = (val - nullzone) / (1.0f - nullzone);
	}
	else if (val < -nullzone) {
		val = (val + nullzone) / (1.0f - nullzone);
	}
	else {
		val = 0.0f;
	}
	val = ctldev->sensmod[axis] * ctldev->sens[axis] * val;
	val = val + 1.0f;

	if (val < 0.0f) val = 0.0f;
	if (val > 2.0f) val = 2.0f;

	// determine value based off requested format.		
	if (format == ctDigital) {
		if (val < 0.5) val = (float)0.0;
		else val = (float)1.0;
	}
	else if (format == ctAnalog) {
		val = val - (float)1.0;
	}
	else {
		val = (float)0.0;
		mprintf((1, "gameController::axis unsupported format for function.\n"));
	}

	ct_packet key_slide1, key_bank;

	get_packet(ctfTOGGLE_SLIDEKEY, &key_slide1);
	get_packet(ctfTOGGLE_BANKKEY, &key_bank);

	if (key_slide1.value || key_bank.value)
	{
		//Don't do mouse look if either toggle is happening
		return val;
	}
	if ((Current_pilot.mouselook_control) && (GAME_MODE == GetFunctionMode()))
	{
		//Don't do mouselook controls if they aren't enabled in multiplayer
		if ((Game_mode & GM_MULTI) && (!(Netgame.flags & NF_ALLOW_MLOOK)))
			return val;

		//Account for guided missile control
		if (Players[Player_num].guided_obj)
			return val;
		axis++;

		if ((axis == CT_X_AXIS) && (ctldev->id == CTID_MOUSE) && (val != 0.0f))
		{
			matrix orient;

			if (!(Players[Player_num].controller_bitflags & PCBF_HEADINGLEFT))
			{
				if (val < 0)
					val = 0.0f;
			}
			if (!(Players[Player_num].controller_bitflags & PCBF_HEADINGRIGHT))
			{
				if (val > 0)
					val = 0.0f;
			}

			if (invert)
				val = -val;

			vm_AnglesToMatrix(&orient, 0.0, val * (((float)(65535.0f / 20)) * .5), 0.0);

			Objects[Players[Player_num].objnum].orient = Objects[Players[Player_num].objnum].orient * orient;

			vm_Orthogonalize(&Objects[Players[Player_num].objnum].orient);
			ObjSetOrient(&Objects[Players[Player_num].objnum], &Objects[Players[Player_num].objnum].orient);
			return 0;

		}
		if ((axis == CT_Y_AXIS) && (ctldev->id == CTID_MOUSE) && (val != 0.0f))
		{
			matrix orient;

			if (!(Players[Player_num].controller_bitflags & PCBF_PITCHUP))
			{
				if (val < 0)
					val = 0.0f;
			}
			if (!(Players[Player_num].controller_bitflags & PCBF_PITCHDOWN))
			{
				if (val > 0)
					val = 0.0f;
			}

			if (invert)
				val = -val;

			vm_AnglesToMatrix(&orient, val * (((float)(65535.0f / 20)) * .5), 0.0, 0.0);

			Objects[Players[Player_num].objnum].orient = Objects[Players[Player_num].objnum].orient * orient;

			vm_Orthogonalize(&Objects[Players[Player_num].objnum].orient);
			ObjSetOrient(&Objects[Players[Player_num].objnum], &Objects[Players[Player_num].objnum].orient);
			return 0;
		}

	}


	return val;
}

float gameWinController::get_button_value(sbyte controller, ct_format format, ubyte button)
{
	float val = (float)0.0;

	if (controller < 0 || controller >= CT_MAX_CONTROLLERS) 
	{
		return 0.0f;
	}
	if (m_ControlList[controller].id == CTID_INVALID) 
	{
		return 0.0f;
	}

#ifdef _DEBUG
	if (m_ControlList[controller].id == CTID_KEYBOARD) 
	{
		Int3();
		return 0.0f;
	}
#endif

	if (button == NULL_BINDING) 
	{
		return val;
	}

	//	buttons are idenitifed as 0=none, 1 = button 1, etc.  so if we have a valid button, then 
	//	decrement counter.
	button--;

	// verify valid button.
	if ((unsigned)button >= m_ControlList[controller].buttons)
		return val;

	switch (format)
	{
		//	note we take care of mouse controls and external controls here
	case ctDownCount:
		if (m_ControlList[controller].id == CTID_MOUSE) 
		{
			val = (float)ddio_MouseBtnDownCount(button);
		}
		else 
		{
			val = (float)m_ExtCtlStates[m_ControlList[controller].id].btnpresses[button];
			m_ExtCtlStates[m_ControlList[controller].id].btnpresses[button] = 0;
		}
		break;

	case ctTime:
		if (m_ControlList[controller].id == CTID_MOUSE) 
		{
			val = ddio_MouseBtnDownTime(button);
		}
		else {
			if (!(m_ExtCtlStates[m_ControlList[controller].id].buttons & (1 << button))) 
			{
				val = m_ExtCtlStates[m_ControlList[controller].id].btntimes[button];
				m_ExtCtlStates[m_ControlList[controller].id].btnstarts[button] = (float)0.0;
				m_ExtCtlStates[m_ControlList[controller].id].btntimes[button] = (float)0.0;
			}
			else 
			{
				val = WinControllerTimer - m_ExtCtlStates[m_ControlList[controller].id].btnstarts[button];
				m_ExtCtlStates[m_ControlList[controller].id].btnstarts[button] = WinControllerTimer;
				m_ExtCtlStates[m_ControlList[controller].id].btntimes[button] = (float)0.0;
			}
		}
		break;

	case ctDigital:
		if (m_ControlList[controller].id == CTID_MOUSE) 
		{
			if (m_MseState.m_buttonMask & (1 << button))
				val = 1.0f;
		}
		else if (m_ExtCtlStates[m_ControlList[controller].id].buttons & (1 << button)) {
			val = 1.0f;
		}
		break;

	default:
		mprintf((1, "gameController::button unsupported format for function\n"));
	}

	return val;
}

float gameWinController::get_pov_value(sbyte controller, ct_format format, ubyte pov_number, ubyte pov)
{
	float val = (float)0.0;

	if (controller < 0 || controller >= CT_MAX_CONTROLLERS) 
	{
		return val;
	}
	if (m_ControlList[controller].id == CTID_INVALID) 
	{
		return val;
	}
#ifdef _DEBUG
	if (m_ControlList[controller].id == CTID_KEYBOARD) 
	{
		Int3();
		return 0.0f;
	}
#endif
	if (!(m_ControlList[controller].flags & (CTF_POV << pov_number))) 
	{
		return val;
	}

	int pov_index = pov / (JOYPOV_MAXVAL / JOYPOV_DIR);
	int cur_pov_index = m_ExtCtlStates[m_ControlList[controller].id].pov[pov_number] / (JOYPOV_MAXVAL / JOYPOV_DIR);

	switch (format)
	{
		//	note we take care of mouse controls and external controls here
	case ctDownCount:
		if (pov_index == JOYPOV_DIR)
			val = 0.0f;
		else {
			val = m_ExtCtlStates[m_ControlList[controller].id].povpresses[pov_number][pov_index];
			m_ExtCtlStates[m_ControlList[controller].id].povpresses[pov_number][pov_index] = 0;
		}
		break;

	case ctDigital:
	{
		if (m_ExtCtlStates[m_ControlList[controller].id].pov[pov_number] == JOYPOV_CENTER)
			val = 0.0f;
		else 	if ((cur_pov_index == 0 || cur_pov_index == 1 || cur_pov_index == 7) && (pov == JOYPOV_UP))
			val = 1.0f;
		else 	if ((cur_pov_index == 1 || cur_pov_index == 2 || cur_pov_index == 3) && (pov == JOYPOV_RIGHT))
			val = 1.0f;
		else 	if ((cur_pov_index == 3 || cur_pov_index == 4 || cur_pov_index == 5) && (pov == JOYPOV_DOWN))
			val = 1.0f;
		else 	if ((cur_pov_index == 5 || cur_pov_index == 6 || cur_pov_index == 7) && (pov == JOYPOV_LEFT))
			val = 1.0f;
		break;
	}

	case ctTime:
		if (cur_pov_index == pov_index) 
		{
			val = WinControllerTimer - m_ExtCtlStates[m_ControlList[controller].id].povstarts[pov_number][pov_index];
			m_ExtCtlStates[m_ControlList[controller].id].povstarts[pov_number][pov_index] = WinControllerTimer;
			m_ExtCtlStates[m_ControlList[controller].id].povtimes[pov_number][pov_index] = 0.0f;
		}
		else 
		{
			val = m_ExtCtlStates[m_ControlList[controller].id].povtimes[pov_number][pov_index];
			m_ExtCtlStates[m_ControlList[controller].id].povstarts[pov_number][pov_index] = 0.0f;
			m_ExtCtlStates[m_ControlList[controller].id].povtimes[pov_number][pov_index] = 0.0f;
		}
		break;

	default:
		mprintf((1, "gameController::pov unsupported format for function\n"));
	}

	return val;
}

float gameWinController::get_key_value(int key, ct_format format)
{
	float val = (float)0.0;

	ASSERT(key < DDIO_MAX_KEYS);

	switch (format)
	{
		//	note we take care of mouse controls and external controls here
	case ctDigital:
		if (KEY_STATE(key))
			val = 1.0f;
		break;

	case ctDownCount:
		val = (float)ddio_KeyDownCount(key);
		break;

	case ctTime:
		val = (float)ddio_KeyDownTime(key);
		break;

	default:
		mprintf((1, "gameController::key unsupported format for function\n"));
	}

	return val;
}

void gameWinController::parse_ctl_file(int devnum, const char* ctlname)
{
}

void gameWinController::extctl_getpos(int id)
{
	tJoyPos ji;
	float timer_val;
	int i;

	if (!m_JoyActive) 
	{
		return;
	}

	timer_val = timer_GetTime();

	joy_GetRawPos((tJoystick)id, &ji);

	//if(g_accum_frame_time == 0.0f) {
	m_ExtCtlStates[id].x = (int)ji.x;
	m_ExtCtlStates[id].y = (int)ji.y;
	m_ExtCtlStates[id].z = (int)ji.z;
	m_ExtCtlStates[id].r = (int)ji.r;
	m_ExtCtlStates[id].u = (int)ji.u;
	m_ExtCtlStates[id].v = (int)ji.v;

	for (i = 0; i < JOYPOV_NUM; i++)
	{
		m_ExtCtlStates[id].last_pov[i] = m_ExtCtlStates[id].pov[i];
		m_ExtCtlStates[id].pov[i] = ji.pov[i];

		//	when pov changes position and new position is not in center, then set a new start time.
		int pov_index = m_ExtCtlStates[id].pov[i] / (JOYPOV_MAXVAL / JOYPOV_DIR);
		int last_pov_index = m_ExtCtlStates[id].last_pov[i] / (JOYPOV_MAXVAL / JOYPOV_DIR);

		if (m_ExtCtlStates[id].pov[i] != m_ExtCtlStates[id].last_pov[i])
		{
			if (m_ExtCtlStates[id].pov[i] != JOYPOV_CENTER)
				m_ExtCtlStates[id].povstarts[i][pov_index] = timer_val;
			if (m_ExtCtlStates[id].last_pov[i] != JOYPOV_CENTER)
				m_ExtCtlStates[id].povtimes[i][last_pov_index] = timer_val - m_ExtCtlStates[id].povstarts[i][last_pov_index];
			m_ExtCtlStates[id].povpresses[i][pov_index]++;
		}

		if (m_ExtCtlStates[id].pov[i] != JOYPOV_CENTER)
		{
			m_ExtCtlStates[id].povtimes[i][pov_index] = timer_val - m_ExtCtlStates[id].povstarts[i][pov_index];
		}

	}
	//}

//	handle buttons
	for (int i = 0; i < CT_MAX_BUTTONS; i++)
	{
		//	case if we read time before doing this again.
		if ((ji.buttons & (1 << i)) && (m_ExtCtlStates[id].btnstarts[i] == (float)0.0))
			m_ExtCtlStates[id].btnstarts[i] = timer_val;
		if ((ji.buttons & (1 << i)) && !(m_ExtCtlStates[id].buttons & (1 << i))) {
			m_ExtCtlStates[id].btnpresses[i]++;
			m_ExtCtlStates[id].btnstarts[i] = timer_val;
			//	mprintf((0, "Start time for %d = %f\n", i, timer_val));
		}

		if (ji.buttons & (1 << i))						// if button is down
			m_ExtCtlStates[id].btntimes[i] = timer_val - m_ExtCtlStates[id].btnstarts[i];
		else if (m_ExtCtlStates[id].buttons & (1 << i))	// if button is up and last pass it was down.
			m_ExtCtlStates[id].btntimes[i] = timer_val - m_ExtCtlStates[id].btnstarts[i];
	}

	m_ExtCtlStates[id].buttons = ji.buttons;
}

void gameWinController::extctl_geteval(int id)
{
}

void gameWinController::mouse_geteval()
{
	int x, y, dx, dy;	//,z;
	unsigned btnmask;

	if (!m_MouseActive) {
		return;
	}

	if (g_accum_frame_time != 0.0f) return;

	btnmask = (unsigned)ddio_MouseGetState(&x, &y, &dx, &dy);

	m_MseState.m_deltaX = dx;
	m_MseState.m_deltaY = dy;
	m_MseState.m_deltaZ = 0;
	m_MseState.m_absX = x;
	m_MseState.m_absY = y;

	m_MseState.m_buttonMask = btnmask;
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

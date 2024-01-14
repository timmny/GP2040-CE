export default {
	'header-text': 'Pin Mapping',
	'sub-header-text': `Here you can configure what pin has what action. If you're unsure what button is connect to what pin, try out the pin viewer.`,
	'alert-text':
		"Mapping buttons to pins that aren't connected or available can leave the device in non-functional state. To clear the invalid configuration go to the <2>Reset Settings</2> page.",
	'pin-viewer': 'Pin viewer',
	'pin-pressed': 'Pressed pin: {{pressedPin}}',
	'pin-header-label': 'Pin',
	'profile-pins-warning':
		'Try to avoid changing the buttons and/or directions used for the switch profile hotkeys. Otherwise, it will be difficult to understand what profile is being selected!',
	errors: {
		conflict: 'Pin {{pin}} is already assigned to {{conflictedMappings}}',
		required: '{{button}} is required',
		invalid: '{{pin}} is invalid for this board',
		used: '{{pin}} is already assigned to another feature',
	},
	'all-capture-button-label': 'Assign Gamepad Pins\u00A0\u00A0🎮',
	actions: {
		NONE: 'None',
		RESERVED: 'Reserved',
		ASSIGNED_TO_ADDON: 'Assigned to addon',
		BUTTON_PRESS_UP: 'Up',
		BUTTON_PRESS_DOWN: 'Down',
		BUTTON_PRESS_LEFT: 'Left',
		BUTTON_PRESS_RIGHT: 'Right',
		BUTTON_PRESS_B1: 'B1',
		BUTTON_PRESS_B2: 'B2',
		BUTTON_PRESS_B3: 'B3',
		BUTTON_PRESS_B4: 'B4',
		BUTTON_PRESS_L1: 'L1',
		BUTTON_PRESS_R1: 'R1',
		BUTTON_PRESS_L2: 'L2',
		BUTTON_PRESS_R2: 'R2',
		BUTTON_PRESS_S1: 'S1',
		BUTTON_PRESS_S2: 'S2',
		BUTTON_PRESS_A1: 'A1',
		BUTTON_PRESS_A2: 'A2',
		BUTTON_PRESS_L3: 'L3',
		BUTTON_PRESS_R3: 'R3',
		BUTTON_PRESS_FN: 'Function',
		BUTTON_PRESS_DDI_UP: 'DDI Up',
		BUTTON_PRESS_DDI_DOWN: 'DDI Down',
		BUTTON_PRESS_DDI_LEFT: 'DDI Left',
		BUTTON_PRESS_DDI_RIGHT: 'DDI Right',
		SUSTAIN_DP_MODE_DP: 'D-Pad Mode: D-Pad',
		SUSTAIN_DP_MODE_LS: 'D-Pad Mode: Left Stick',
		SUSTAIN_DP_MODE_RS: 'D-Pad Mode: Right Stick',
		SUSTAIN_SOCD_MODE_UP_PRIO: 'Up Priority SOCD Cleaning',
		SUSTAIN_SOCD_MODE_NEUTRAL: 'Neutral SOCD Cleaning',
		SUSTAIN_SOCD_MODE_SECOND_WIN: 'Last Win SOCD Cleaning',
		SUSTAIN_SOCD_MODE_FIRST_WIN: 'First Win SOCD Cleaning',
		SUSTAIN_SOCD_MODE_BYPASS: 'SOCD Cleaning Off',
	},
};

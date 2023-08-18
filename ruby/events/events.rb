require 'ruby/aerosol.rb'

module AS::Event
	class BaseEvent
		attr_reader :source_id
		
		def any_key?
			false
		end
	end
	
  class KeyboardEvent < BaseEvent
    attr_reader :key, :modifiers, :state
    
    def self.source_id(key)
      (key << 3) | 1
    end
  
    def inspect
      "KeyEvent:(#{Keynames[@key]}) state:#{@state}"
    end
  
    # Create a new keyboard event.
    # state: true signified key pressed, false is for the key release event.
    # key: which key
    # modifiers: list of modifier keys that were also pressed, like shift.
    def initialize(state, key, modifiers = nil)
      @key = key
      @modifiers = modifiers
      @state = state
      
      @source_id = self.class.source_id(key)
    end
		
		def any_key?
			@state == true
		end
  end
  
  class MouseMotionEvent < BaseEvent
    attr_reader :pos, :delta_pos
    
    def self.source_id
      3
    end
  
    def inspect
      "MouseMotionEvent: pos#{@pos.inspect} delta#{@delta_pos.inspect}"
    end
  
    # The mouse moved
    def initialize(pos, delta_pos)
      @pos = pos
      @delta_pos = delta_pos
      
      @source_id = self.class.source_id
    end
  end
  
  class MouseButtonEvent < BaseEvent
    attr_reader :state, :button, :pos
    
    def self.source_id(button)
      (button << 3) | 2
    end
  
    def inspect
      "MouseButtonEvent: button:#{@button}, state:#{@state} pos#{@pos.inspect}"
    end
  
    def initialize(state, button, pos)
      @state = state
      @button = button
      @pos = pos
      
      @source_id = self.class.source_id(button)
    end

		def any_key?
			@state == true
		end
  end
	
	def self.source_for_key(key)
		sym = case key
			when Symbol: KeyboardEvent::Keysyms[key]
			when Fixnum: key
		end
		
		raise "No such key" unless KeyboardEvent::Keynames.has_key? sym
		
		KeyboardEvent.source_id(sym)
	end
	
	def self.source_for_mouse
		MouseMotionEvent.source_id
	end
	
	def self.source_for_mbutton(button)
		MouseButtonEvent.source_id(button)
	end
end

class AS::Event::ActionListener
	def initialize(parent = nil)
		@parent = parent
		@actions = Hash.new([])
	end
	
	def action(event_id, &block)
		# create a new array with these two appended together, and assign it to the list of actions for this event_id
		@actions[event_id] += [block]
	end
	
	def action_for_key(*keys, &block)
		keys.each{|key|
			sym = case key
				when Symbol: AS::Event::KeyboardEvent::Keysyms[key]
				when Fixnum: key
			end
			
			raise "No such key if" if AS::Event::KeyboardEvent::Keynames[sym] == 'Unknown'
			
			action(AS::Event::KeyboardEvent.source_id(sym), &block)
		}
	end
	
	def call(event)
		if @actions.key?(event.source_id)
			@actions[event.source_id].each{|a|
				a.call(event)
			}
			
			return true
		elsif @parent
			return @parent.call(event)
		end
		
		return false
	end
end

class AS::Event::KeyboardEvent
	# Hash of all key names and their SDL keysyms.
	Keysyms = {
		:Key_UNKNOWN			=> 0,
		:Key_FIRST				=> 0,
		:Key_BACKSPACE		=> 8,
		:Key_TAB					=> 9,
		:Key_CLEAR				=> 12,
		:Key_RETURN				=> 13,
		:Key_PAUSE				=> 19,
		:Key_ESCAPE				=> 27,
		:Key_SPACE				=> 32,
		:Key_EXCLAIM			=> 33,
		:Key_QUOTEDBL			=> 34,
		:Key_HASH					=> 35,
		:Key_DOLLAR				=> 36,
		:Key_AMPERSAND		=> 38,
		:Key_QUOTE				=> 39,
		:Key_LEFTPAREN		=> 40,
		:Key_RIGHTPAREN		=> 41,
		:Key_ASTERISK			=> 42,
		:Key_PLUS					=> 43,
		:Key_COMMA				=> 44,
		:Key_MINUS				=> 45,
		:Key_PERIOD				=> 46,
		:Key_SLASH				=> 47,
		:Key_0						=> 48,
		:Key_1						=> 49,
		:Key_2						=> 50,
		:Key_3						=> 51,
		:Key_4						=> 52,
		:Key_5						=> 53,
		:Key_6						=> 54,
		:Key_7						=> 55,
		:Key_8						=> 56,
		:Key_9						=> 57,
		:Key_COLON				=> 58,
		:Key_SEMICOLON		=> 59,
		:Key_LESS					=> 60,
		:Key_EQUALS				=> 61,
		:Key_GREATER			=> 62,
		:Key_QUESTION			=> 63,
		:Key_AT						=> 64,
		:Key_LEFTBRACKET	=> 91,
		:Key_BACKSLASH		=> 92,
		:Key_RIGHTBRACKET => 93,
		:Key_CARET				=> 94,
		:Key_UNDERSCORE		=> 95,
		:Key_BACKQUOTE		=> 96,
		:Key_a						=> 97,
		:Key_b						=> 98,
		:Key_c						=> 99,
		:Key_d						=> 100,
		:Key_e						=> 101,
		:Key_f						=> 102,
		:Key_g						=> 103,
		:Key_h						=> 104,
		:Key_i						=> 105,
		:Key_j						=> 106,
		:Key_k						=> 107,
		:Key_l						=> 108,
		:Key_m						=> 109,
		:Key_n						=> 110,
		:Key_o						=> 111,
		:Key_p						=> 112,
		:Key_q						=> 113,
		:Key_r						=> 114,
		:Key_s						=> 115,
		:Key_t						=> 116,
		:Key_u						=> 117,
		:Key_v						=> 118,
		:Key_w						=> 119,
		:Key_x						=> 120,
		:Key_y						=> 121,
		:Key_z						=> 122,
		:Key_DELETE				=> 127,
		:Key_WORLD_0			=> 160,
		:Key_WORLD_1			=> 161,
		:Key_WORLD_2			=> 162,
		:Key_WORLD_3			=> 163,
		:Key_WORLD_4			=> 164,
		:Key_WORLD_5			=> 165,
		:Key_WORLD_6			=> 166,
		:Key_WORLD_7			=> 167,
		:Key_WORLD_8			=> 168,
		:Key_WORLD_9			=> 169,
		:Key_WORLD_10			=> 170,
		:Key_WORLD_11			=> 171,
		:Key_WORLD_12			=> 172,
		:Key_WORLD_13			=> 173,
		:Key_WORLD_14			=> 174,
		:Key_WORLD_15			=> 175,
		:Key_WORLD_16			=> 176,
		:Key_WORLD_17			=> 177,
		:Key_WORLD_18			=> 178,
		:Key_WORLD_19			=> 179,
		:Key_WORLD_20			=> 180,
		:Key_WORLD_21			=> 181,
		:Key_WORLD_22			=> 182,
		:Key_WORLD_23			=> 183,
		:Key_WORLD_24			=> 184,
		:Key_WORLD_25			=> 185,
		:Key_WORLD_26			=> 186,
		:Key_WORLD_27			=> 187,
		:Key_WORLD_28			=> 188,
		:Key_WORLD_29			=> 189,
		:Key_WORLD_30			=> 190,
		:Key_WORLD_31			=> 191,
		:Key_WORLD_32			=> 192,
		:Key_WORLD_33			=> 193,
		:Key_WORLD_34			=> 194,
		:Key_WORLD_35			=> 195,
		:Key_WORLD_36			=> 196,
		:Key_WORLD_37			=> 197,
		:Key_WORLD_38			=> 198,
		:Key_WORLD_39			=> 199,
		:Key_WORLD_40			=> 200,
		:Key_WORLD_41			=> 201,
		:Key_WORLD_42			=> 202,
		:Key_WORLD_43			=> 203,
		:Key_WORLD_44			=> 204,
		:Key_WORLD_45			=> 205,
		:Key_WORLD_46			=> 206,
		:Key_WORLD_47			=> 207,
		:Key_WORLD_48			=> 208,
		:Key_WORLD_49			=> 209,
		:Key_WORLD_50			=> 210,
		:Key_WORLD_51			=> 211,
		:Key_WORLD_52			=> 212,
		:Key_WORLD_53			=> 213,
		:Key_WORLD_54			=> 214,
		:Key_WORLD_55			=> 215,
		:Key_WORLD_56			=> 216,
		:Key_WORLD_57			=> 217,
		:Key_WORLD_58			=> 218,
		:Key_WORLD_59			=> 219,
		:Key_WORLD_60			=> 220,
		:Key_WORLD_61			=> 221,
		:Key_WORLD_62			=> 222,
		:Key_WORLD_63			=> 223,
		:Key_WORLD_64			=> 224,
		:Key_WORLD_65			=> 225,
		:Key_WORLD_66			=> 226,
		:Key_WORLD_67			=> 227,
		:Key_WORLD_68			=> 228,
		:Key_WORLD_69			=> 229,
		:Key_WORLD_70			=> 230,
		:Key_WORLD_71			=> 231,
		:Key_WORLD_72			=> 232,
		:Key_WORLD_73			=> 233,
		:Key_WORLD_74			=> 234,
		:Key_WORLD_75			=> 235,
		:Key_WORLD_76			=> 236,
		:Key_WORLD_77			=> 237,
		:Key_WORLD_78			=> 238,
		:Key_WORLD_79			=> 239,
		:Key_WORLD_80			=> 240,
		:Key_WORLD_81			=> 241,
		:Key_WORLD_82			=> 242,
		:Key_WORLD_83			=> 243,
		:Key_WORLD_84			=> 244,
		:Key_WORLD_85			=> 245,
		:Key_WORLD_86			=> 246,
		:Key_WORLD_87			=> 247,
		:Key_WORLD_88			=> 248,
		:Key_WORLD_89			=> 249,
		:Key_WORLD_90			=> 250,
		:Key_WORLD_91			=> 251,
		:Key_WORLD_92			=> 252,
		:Key_WORLD_93			=> 253,
		:Key_WORLD_94			=> 254,
		:Key_WORLD_95			=> 255,
		:Key_KP0					=> 256,
		:Key_KP1					=> 257,
		:Key_KP2					=> 258,
		:Key_KP3					=> 259,
		:Key_KP4					=> 260,
		:Key_KP5					=> 261,
		:Key_KP6					=> 262,
		:Key_KP7					=> 263,
		:Key_KP8					=> 264,
		:Key_KP9					=> 265,
		:Key_KP_PERIOD		=> 266,
		:Key_KP_DIVIDE		=> 267,
		:Key_KP_MULTIPLY	=> 268,
		:Key_KP_MINUS			=> 269,
		:Key_KP_PLUS			=> 270,
		:Key_KP_ENTER			=> 271,
		:Key_KP_EQUALS		=> 272,
		:Key_UP						=> 273,
		:Key_DOWN					=> 274,
		:Key_RIGHT				=> 275,
		:Key_LEFT					=> 276,
		:Key_INSERT				=> 277,
		:Key_HOME					=> 278,
		:Key_END					=> 279,
		:Key_PAGEUP				=> 280,
		:Key_PAGEDOWN			=> 281,
		:Key_F1						=> 282,
		:Key_F2						=> 283,
		:Key_F3						=> 284,
		:Key_F4						=> 285,
		:Key_F5						=> 286,
		:Key_F6						=> 287,
		:Key_F7						=> 288,
		:Key_F8						=> 289,
		:Key_F9						=> 290,
		:Key_F10					=> 291,
		:Key_F11					=> 292,
		:Key_F12					=> 293,
		:Key_F13					=> 294,
		:Key_F14					=> 295,
		:Key_F15					=> 296,
		:Key_NUMLOCK			=> 300,
		:Key_CAPSLOCK			=> 301,
		:Key_SCROLLOCK		=> 302,
		:Key_RSHIFT				=> 303,
		:Key_LSHIFT				=> 304,
		:Key_RCTRL				=> 305,
		:Key_LCTRL				=> 306,
		:Key_RALT					=> 307,
		:Key_LALT					=> 308,
		:Key_RMETA				=> 309,
		:Key_LMETA				=> 310,
		:Key_LSUPER				=> 311,
		:Key_RSUPER				=> 312,
		:Key_MODE					=> 313,
		:Key_COMPOSE			=> 314,
		:Key_HELP					=> 315,
		:Key_PRINT				=> 316,
		:Key_SYSREQ				=> 317,
		:Key_BREAK				=> 318,
		:Key_MENU					=> 319,
		:Key_POWER				=> 320,
		:Key_EURO					=> 321,
		:Key_UNDO					=> 322,
	}
	Keysyms.default = 0

	# Inverse of Keysyms.
	Keynames = Keysyms.invert
	Keynames.default = "Unknown"
end

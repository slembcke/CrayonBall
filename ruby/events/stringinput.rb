# Handles inputing strings using Aerosol events.
class AS::Event::StringInput
	
	BACKSPACE = AS::Event::KeyboardEvent::Keysyms[:Key_BACKSPACE]
	RETURN = AS::Event::KeyboardEvent::Keysyms[:Key_RETURN]
	LSHIFT = AS::Event::KeyboardEvent::Keysyms[:Key_LSHIFT]
	RSHIFT = AS::Event::KeyboardEvent::Keysyms[:Key_RSHIFT]
	
	down = %q{abcdefghijklmnopqrstuvwxyz`1234567890-=[]\;',./ }.unpack("c*") #`
	up =   %q[ABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()_+{}|:"<>? ].unpack("c*") #"
	UPCASE = Hash[*down.zip(up).flatten]
	
	def initialize(str, max_length)
		@render_string = str
		@working_string = str.clone
		@max_length = max_length
	end
	
	def shift_char(char, modifiers)
		return '' unless UPCASE.has_key?(char)
		if modifiers.include?(:SHIFT)
			return UPCASE[char].chr
		else
			return char.chr
		end
	end

	attr_reader :render_string

	private :shift_char
	
	# Pass an event.
	def call(event)
		return unless event.class == AS::Event::KeyboardEvent 
		return if !event.state
		
		key = event.key
		
		case key
		when BACKSPACE: @working_string.chop!
		when RETURN: finalize_string
		else 
			return if @working_string.length == @max_length
			@working_string.concat(shift_char(key, event.modifiers))
		end
		
		update_render_string
	end
	
	# Usually used internally. Accepts the string input after yout hit return. 
	# You might call this if the user hits an 'accept' button, as well.
	def finalize_string
		@final = @working_string.clone
	end
	
	def update_render_string
			@render_string.replace(@working_string + @caret.to_s)
	end	
	
	# The string that has been entered, updated after the user presses return.
	def final_string
		@final
	end
	
	def toggle_caret(caret_str = "|")
		if @caret
			 @caret = nil
		else
			 @caret = caret_str
		end
		update_render_string
	end
end

#one day will be promoted to Aerosol proper, if determined worthy.

def puts(value)
	return if !DEBUG_MODE
	where = caller[0].sub(AS::LIB_DIR, "$(AS::LIB_DIR)")
	Kernel.puts("\033[31m#{where}:\033[0m #{value}")
end

def p(value)
	return if !DEBUG_MODE
	where = caller[0].sub(AS::LIB_DIR, "$(AS::LIB_DIR)")
	Kernel.puts("\033[31m#{where}:\033[0m #{value.inspect}")
end

class ScreenCopyTexture < Photon::Texture::Rect
	def initialize(filter = :smooth, inform = :rgba)
		empty_image = AS::Image.new(nil, AS.screen_w, AS.screen_h, nil)
		init(empty_image, filter, inform, false)
	end
	
	def upload
		#copy pixels from the framebuffer instead of the image
		GL.CopyTexImage2D(tex_target, 0, @inform, 0, 0, @w, @h, 0)
	end
end
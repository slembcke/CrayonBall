AS::Image = Struct.new(:data, :w, :h, :format)

class AS::Image

	# Load an image file. Accepts either PNG or JPEG files. PNG files are
	# loaded with pre-multiplied alpha.
	def self.load_img(filename)
		case extension = File.extname(filename)
		when '.png': AS::Image.load_png_rgba_premult(filename)
		when '.jpg': AS::Image.load_jpeg_rgb(filename)
		else raise "Unknown extension #{extension}"
		end
	end

end
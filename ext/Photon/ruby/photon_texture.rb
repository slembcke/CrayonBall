module Photon
	class Texture
		attr_reader :w, :h, :tex_id
		
		# Possible _filter_ parameters.
		Filter = {
			:smooth  => [GL::LINEAR, GL::LINEAR],
			:blocky => [GL::NEAREST, GL::NEAREST],
			:mip_smooth => [GL::LINEAR_MIPMAP_LINEAR, GL::LINEAR],
			:mip_blocky => [GL::NEAREST_MIPMAP_LINEAR, GL::LINEAR],
		}
		
		# Possible _inform_ parameters
		Format = {
			:alpha => GL::ALPHA,
			:bw    => GL::LUMINANCE,
			:rgb   => GL::RGB,
			:rgba  => GL::RGBA,
		}
		
		def pow2?(num)
			0.upto(16){|n| return true if num == 2**n}
			return false
		end
		
		class DimError < Exception
		end
	
		def validate(image)
			unless pow2?(image.w) and pow2?(image.h)
				raise(DimError, "Width or height not a power of 2")
			end
		end
		
		def self.finalizer_block(id)
			lambda{GL.DeleteTextures([id])}
		end
		
		def gen_tex_id
			@tex_id = GL.GenTextures(1).first
			ObjectSpace.define_finalizer(self, Texture.finalizer_block(@tex_id))
		end
		
		def release
			self.class.finalizer_block(@tex_id).call
			ObjectSpace.undefine_finalizer(self)
		end
		
		def load
			gen_tex_id
			set_data(tex_target, tex_id)
			
			GL.BindTexture(tex_target, tex_id)
		
			GL.TexParameter(tex_target, GL::TEXTURE_WRAP_S, @repeat)
			GL.TexParameter(tex_target, GL::TEXTURE_WRAP_T, @repeat)
			minf, magf = *@filter
			GL.TexParameter(tex_target, GL::TEXTURE_MIN_FILTER, minf)
			GL.TexParameter(tex_target, GL::TEXTURE_MAG_FILTER, magf)
			GL.TexEnv(GL::TEXTURE_ENV, GL::TEXTURE_ENV_MODE, GL::MODULATE)
			
			upload
						
			return self
		end
		
		def upload
			GL.TexImage2D(tex_target, 0, @inform, @w, @h, 0, @pix_format, GL::UNSIGNED_BYTE, @pixels)
		end
		
		def init(image, filter, inform, repeat)
			image = AS::Image.load_img(image) if image.is_a? String
			
			validate(image)
			@pixels = image.data
			@w = image.w
			@h = image.h
			@pix_format = image.format
			
			@filter = case filter
			when Symbol: Texture::Filter[filter]
			when Array: filter
			end
		
			raise(ArgumentError, "#{filter} is not a valid texture filter") unless @filter    
	
			@repeat = case repeat
			when true: GL::REPEAT
			when false: 0x812F
			when Fixnum: repeat
			else raise(ArgumentError, "repeat must be a GL constant or boolean.")
			end
			
			@inform = case inform
			when Symbol: Texture::Format[inform]
			when Fixnum: inform
			end
			
			raise(ArgumentError, "#{inform} is not a valid texture format") unless @inform
			
			load
		end
		
		class TwoD < Texture
			def initialize(image, filter = :smooth, inform = :rgba, repeat = false)
				init(image, filter, inform, repeat)
			end
			
			def tex_target
				GL::TEXTURE_2D
			end
		end
		
		class MIP < Texture
			def initialize(image, filter = :mip_smooth, inform = :rgba, repeat = false)
				init(image, filter, inform, repeat)
			end
			
			def tex_target
				GL::TEXTURE_2D
			end
			
			def upload
				GLU.Build2DMipmaps(tex_target, @inform, @w, @h, @pix_format, GL::UNSIGNED_BYTE, @pixels)
			end
		end
		
		class Rect < Texture
			def initialize(image, filter = :smooth, inform = :rgba)
				init(image, filter, inform, false)
			end
			
			def validate(image)
				true
			end
			
			def tex_target
				GL::TEXTURE_RECTANGLE_EXT
			end
		end
	end
	
	module TextureLoader
		AS::ResourceCache::Loaders << self
		
		def self.match?(key)
			/.*\.png/ === key
		end
		
		def self.loader
			lambda{|key| puts "loading #{key}"; Photon::Texture::Rect.new(key)}
		end
	end
end

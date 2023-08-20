# The Photon scenegraph library.
module Photon
	# Color values are used by Photon::Node::RenderState nodes.
	# You use them to set the target rendering color of nodes drawn below it in the scenegraph.
	# Color is a very weak attribute as it can be overriden by VAR nodes using a color array.
	class Color
		def to_s
			"#<Photon::Color r=#{r}, g=#{g}, b=#{b}, a=#{a}>"
		end
		
		# Get the red, green, and blue values of the color.
		def to_rgb
			[r, g, b]
		end
		
		# Get the red, green, blue and alpha values of the color.
		def to_rgba
			[r, g, b, a]
		end
	end
	
	# Blend objects are used by Photon::Node::RenderState nodes.
	# Blending modes change how pixels are combined with existing pixels in the framebuffer.
	# See the man page for glBlendFunc for more information.
	class Blend
		States = {}
		
		def self.new(sfactor, dfactor)
			key = [sfactor, dfactor]
			state = States[key]
			
			if state
				retur state
			else
				return States[key] = super(sfactor, dfactor)
			end
		end
	end
	
	# Photon::Node objects represent state changes, transformations or other changes in a drawing tree.
	module Node
	
		# Vertex Arrays are the leaves of the scenegraph. They are responsible for drawing.
		# For helper methods, see Photon::VertexArray#make_node and Photon::VertexArray::Chunk#make_node
		class VAR
			# Define the Photon::VertexArray for a new node in a convenient way.
			def self.ad_hoc(mode, num_vertex, num_color, num_texcoord)
				var = Photon::VertexArray.new(num_vertex, num_color, num_texcoord)
				yield(var)
				
				return var.make_node(mode)
			end
		end
	end
end

require 'chipmunk'
require 'photonEXT'

require 'photon_var.rb'
require 'photon_texture.rb'
require 'photon_font.rb'

# define handy constants
module Photon
	class Color
		Black = self.new(0,0,0)
		White = self.new(1,1,1)

		Red = self.new(1,0,0)
		Green = self.new(0,1,0)
		Blue = self.new(0,0,1)

		Yellow = self.new(1,1,0)
		Magenta = self.new(1,0,1)
		Cyan = self.new(0,1,1)
		
		def initialize_copy(other)
			super
			self.color = other
		end
	end
	
	# An OpenGl blending mode. Common blending modes are defined.
	class Blend
		# Normal alpha blending
		Alpha = self.new(GL::SRC_ALPHA, GL::ONE_MINUS_SRC_ALPHA)
		# Pre-multiplied alpha blending. Aerosol loads textures as pre-multiplied.
		PremultAlpha = self.new(GL::ONE, GL::ONE_MINUS_SRC_ALPHA)
		# Additive blending
		Add = self.new(GL::ONE, GL::ONE)
		# Component-wise color multiplication. Sometimes known as burn.
		Multiply = self.new(GL::DST_COLOR, GL::ZERO)
	end

	module Node
		class CopyRect < RenderState
			def initialize(tex, sx, sy, sw, sh, dx, dy, dw, dh)
				var = VertexArray.for_copy_rect(sx, sy, sw, sh, dx, dy, dw, dh)
				super({:texture => tex}, var.make_node(GL::QUADS))
			end
			
			def self.new_for_texture(tex, dx=0, dy=0, scale=1.0)
				tw, th = (tex.tex_target == GL::TEXTURE_RECTANGLE_EXT) ? [tex.w, tex.h] : [1, 1]
				self.new(tex, 0, 0, tw, th, dx, dy, tex.w * scale, tex.h * scale);
			end
			
		end
		
		# A convenient class for creating a Photon::Node::RenderState and Photon::Node::VAR for drawing a sprite.
		# The geometry will be generated using pixel sized coordinates, (kx, ky) is the keypoint of the sprite.
		class SpriteTemplate < RenderState
			def initialize(tex, kx, ky)
				super({:texture => tex}, VertexArray.for_sprite(tex, kx, ky).make_node(GL::QUADS))
			end
		end
	end
end

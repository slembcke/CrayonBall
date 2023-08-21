module Photon

	# Matrix transformation class.
	# 
	class Matrix
	
		# Given no arguments, creates an identity matrix. Otherwise, it
		# assumes 16 values in column major format (meaning it fills values 
		# down and then across)
		def initialize(*argv); end
	
		# Sets the matrix back to the identity matrix.
		def reset!(); end
		
		# Multiplies self by a 2D translation matrix.
		def translate!(x, y); end

		# Multiplies self by a 2D scale matrix.
		def scale!(x, y); end

		# Multiplies self by a 2D rotation matrix. Value in radians.
		def rotation!(radians); end

		# Multiplies self by a transformation matrix that does a scale, translation and rotate.
		# It translates such that (0,0) becomes (x1, y1) and (1, 0) becomes (x2, y2)
		# Useful for moving a image of a bone into place on a game.
		def bone_scale!(x1, y1, x2, y2); end
	
		# Multiplies self by a translate, scale and then a rotate.
		def sprite!(x, y, scale_x, scale_y, radians); end
	
		# Multiplies self by a orthographic projection matrix. See 'man gluOrtho2D'
		def ortho!(left, right, bottom, top); end		
	
	end

	class Color
		
		# Copmonent values between 0.0 and 1.0.
		def initialize(r, g, b, a = 1.0); end

		# Component between 0.0 and 1.0
		def self.new_grey(v, a = 1.0); end

		# Each component between 0.0 and 1.0
		def self.new_hsv(hue, saturation, value, alpha = 1.0); end

		# Replaces the values in self with the values in the specified color.
		# This is the only way to modify a color.
		def color=(c); end

		# Linear interpolation between self and the specified color. 
		# t=0 is self, t=1 is the other color. t is clamped between 0.0 and 1.0.
		def lerp(t, other_color); end
		
	end

	module Node

		attr_accessor :enabled
	
		# Returns the child node, if applicable, else nil
		def child(); end
	
		# Sets the openGL state to the default and renders the self and the rest of the 
		# scenegraph, treating itself as the root.
		def render(); end	
		
		class VAR
		
			# Create a vertex array with:
			#   A vertex array
			#   A primitive opengl drawing mode (GL::LINES, GL::QUADS, etc)
			#   First index in the vertex array that you want to draw
			#   How many elements of the vertex array to draw
			def initialize(vertex_array, drawing_mode, first_index, count); end
		
		end
	
		# A Photon::Node that changes the openGL state for all child nodes beneath it.
		class RenderState
			
			# An immutable hash of the render state. Note that the :texture and :color elements themselves
			# are mutable.
			attr_reader :state_hash
			
			# Create a RenderState with a hash of parameters:
			#   :color => A Photon::Color
			#   :blend => A Photon::Blend (such as (Photon::Blend::Alpha)
			#   :texture => A Photon::Texture
			# And a child node.
			def initialize(hash, child); end
			
			# By Photon convention, nodes that take a child can pass the child in as the return 
			# value of a attached block. In these cases, use construct instead of new.
			def self.construct(*argv, &block); end
			
		end
	
		# A matrix transformation applied to child nodes.
		class Matrix
			
			attr_accessor :matrix
			
			# Create a matrix node:
			#   mode is GL::TEXTURE, GL::MODELVIEW, GL::PROJECTION
			#     Note that for GL::TEXTURE and GL::PROJECTION, cannot be nested more than other GL::TEXTUREs or other GL::PROJECTION
			#   matrix, a Photon::Matrix
			#   and a child.
			def initialize(mode, matrix, child); end
			
			# By Photon convention, nodes that take a child can pass the child in as the return 
			# value of a attached block. In these cases, use construct instead of new.
			def self.construct(*argv, &block); end
			
		end
		
		# Node for printing pretty text. Use string.replace to 
		class Text
		
			attr_accessor :str
			
			# Create a font node with the Photon::Font and a string.
			def initialize(font, string); end
			
		end
		
		# A node with many children, backed by a ruby array. Does not change the OpenGL state or draw anything by itself.
		class Array
			
			attr_reader :children
			
			# Given a ruby array of child nodes.
			def initialize(array); end
			
		end
		
		# Node that adjusts the position and rotation of children nodes by the
		# position and rotation of a Chipmunk body.
		class Body
			attr_reader :body
			
			def initialize(body, child); end
		
			# By Photon convention, nodes that take a child can pass the child in as the return 
			# value of a attached block. In these cases, use construct instead of new.
			def self.construct(*argv, &block); end

		end

		# Node that adjusts the position of children nodes by the
		# position of a Chipmunk body. Does NOT affect rotation.
		class BodyPos
			attr_reader :body
			
			def initialize(body, child); end
		
			# By Photon convention, nodes that take a child can pass the child in as the return 
			# value of a attached block. In these cases, use construct instead of new.
			def self.construct(*argv, &block); end

		end
	
		# Magical node that renders everything in a chipmunk space.
		class Space
			attr_reader :space
			
			def initialize(space); end
		
			# By Photon convention, nodes that take a child can pass the child in as the return 
			# value of a attached block. In these cases, use construct instead of new.
			def self.construct(*argv, &block); end

		end
	
	end


end
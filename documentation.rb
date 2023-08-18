# Main module for Aerosol
module AS
  # List of attached joysticks as AS::Joysticks.
  Joysticks = nil
  
  # List of all available fullscreen resolutions. Entries are a two
  # element Array. _[w,h]_
  Resolutions = nil

  # TODO image loading/saving funcs
  
  # Swap the OpenGL buffers. (draw the screen)
  def self.swap_buffers; end
  
  # Set the mouse visibility.
  def self.show_mouse=(visibility); end
  
  # Grab exclusive use of the mouse input.
  def self.grab_mouse=(grab); end
  
  # Open a screen. Can be called more than once, but each call will
  # completely reset OpenGL. Opts include :vbl_sync => bool,
  # :fullscreen => bool, :depth => bits, :stencil => bits.
  def self.open_screen(w, h, opts = {}); end
  
  # Current screen width
  def self.screen_w; end
  
  # Current screen height
  def self.screen_h; end
  
  # Should be called before exiting Aerosol. (normally called
  # automatically)
  def self.exit; end
  
  # Clear the screen
  def self.clear_screen; end
  
  # Set the clear color
  def self.clear_color(red, gree, blue, alpha = 1.0); end

  module Event
    # Get queued events from Aerosol, and update the input tables.
    # Passes each in turn to the block.
    def self.each; yeild; end
    
    # Synonym for Event.each.
    def self.pump; yeild; end
    
    # Get the value for key where key is one of the keysyms from
    # Keysyms or the ASCII code given by ?.  Returns true if the key
    # is pressed and false otherwise.
    def self.key_value(key); end
    
    # Get the value for a mouse button.  Returns true if the button is
    # pressed and false otherwise.
    def self.mbutton_value(button); end

    # Get the mouse position. Returns a two element Array.
    def self.mouse_pos; end

    # Get the relative mouse motion. Returns a two element Array.
    def self.mouse_rel; end
    
    # Get the value for a joystick button.  Returns true if the button
    # is pressed and false otherwise.
    def self.jbutton_value(joynum, button); end
    
    # Get the value for a joystick axis.  Returns a Float from [-1, 1]
    def self.jaxis_value(joynum, axis); end
    
    # Geth the value for a hat.  Returns a value that corresponds to
    # the Hat_* constants.
    def self.jhat_value(joynum, hat); end
  end
  
  # Ogg Vorbis audio player
  class Music
    # Open an Ogg Vorbis file.  Loop should be true or false depending
    # on whether the music should loop when complete.
    def initialize(filename, loop); end
    
    # Begin playing.
    def play; end
    
    # Stop and rewind.
    def stop; end
    
    # Stop but don't rewind.
    def pause; end
    
    # Resume a paused music player.
    def resume; end
    
    # Set the volume.
    def volume=(vol); end
  end

  module Scene
    module SceneNode
      # Child of the node.
      attr_reader :child

      # Whether or not to draw the node.
      attr_accessor :enabled

      # Render the graph starting at this node.
      def render; end
    end

    class BlendNode
      include SceneNode

      # _function_ is ignored at the moment. _src_factor_ and
      # _dst_factor_ are arguments to glBlendFunc().
      def initialize(function, src_factor, dst_factor, child); 
      end
    end

    class BodyNode
      include SceneNode
      
      # _body_ is the CP::Body to link the transformation to.
      def initialize(body, child); end
    end

    class ColorNode
      include SceneNode

      # _color_ should be an AS::PackedColor color array.
      def initialize(color, child); end
    end

    class GroupNode
      include SceneNode

      # _children_ should be an array of SceneNode objects. Currently
      # no type checking is done. Be careful!
      def initialize(children); end
    end

    class MatrixNode
      include SceneNode

      # The AS::PackedMatrix associated with this node.
      attr_reader :matrix
      
      # _matrix_mode_ should be a GL matrix mode. (e.g. GL::MODELVIEW).
      # _matrix_ should be a AS::PackedMatrix.
      def initialize(matrix_mode, matrix, child); end
    end

    class RenderNode
      include SceneNode

      # _vbuffer_ should be the AS::VertexBuffer object to bind. _mode_,
      # _index_ and _count_ are arguments to glDrawArrays(). You
      # cannot set a child for a RenderNode.
      def initialize(vbuffer, mode, index, count); end
    end

    class TextureNode
      include SceneNode

      # _texture_ is the AS::Texture object to bind.
      def initialize(texture, child); end
    end
    
    class SpriteNode
    	include SceneNode
    	
    	attr_accessor :pos, :angle, :scale_x, :scale_y
    	
    	def initialize(child); end
    end
    
    class RubyNode
    	include RubyNode
    	
    	attr_reader :delegate
    	
    	# _delegate_ Any object with a #call method that will be invoked
    	# to draw the node. The RubyNode will be passed as a parameter.
    	def initialize(delegate); end
    end
  end
 end
      

class Array
  # Call Object#send(method, *args) on every object in the array.
  # Implemented in C, so this is _much_ faster that using Array#each
  # for simple method calls.
  def send_each(method, *args); end
end

class Float
  # Clamp self to the given range.
  def clamp(min, max); end
  
  def clamp!(min, max); end
  
  # Return 0.0 if self.abs is less than tol.
  def tolerance(tol); end
  
  def tolerance!(tol); end
  
  # Convert from radians to degrees.
  def rad2deg; end
  
  # Convert from degrees to radians.
  def deg2rad; end
  
  # Convert from radians to a normalized vector in that direction.
  def rad2unit_vect; end
  
  # Convert from degrees to a normalized vector in that direction.
  def deg2unit_vect; end
end

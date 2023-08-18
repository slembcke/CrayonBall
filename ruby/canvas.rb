# A class for manipulating the viewport.
# Using a Canvas object you can easily have the view follow a character or rotate or scale the view.
class AS::Canvas
  # Create a new Canvas object.
  # kx and ky are the key point (origin offset).
  def initialize(kx, ky, w, h)
    @kx, @ky, @w, @h = kx, ky, w, h
    @pos = vec2(0, 0)
    @angle = 0.0
    @scale = 1.0
  end
  
  attr_accessor :kx, :ky
  attr_accessor :w, :h
  attr_accessor :pos, :angle, :scale
  
  # Slew to a position.
  # _factor_ designates how much of a change should actually occur.
  def slew_pos(pos, factor)
    @pos += (pos - @pos)*factor
  end
  
  # Slew to an angle.
  # _factor_ designates how much of a change should actually occur.
  def slew_angle(angle, factor)
    @angle += (angle - @angle)*factor
  end
  
  # Slew to a scale.
  # _factor_ designates how much of a change should actually occur.
  def slew_scale(scale, factor)
    @scale *= (scale/@scale)**factor
  end
  
  # Get the minimum x value of the canvas. (Only valid when angle = 0)
  def xmin
    @pos.x - @kx/@scale
  end
  
  # Get the maximum x value of the canvas. (Only valid when angle = 0)
  def xmax
    @pos.x + (@w - @kx)/@scale
  end
  
  # Get the minimum y value of the canvas. (Only valid when angle = 0)
  def ymin
    @pos.y - @ky/@scale
  end
  
  # Get the maximum x value of the canvas. (Only valid when angle = 0)
  def ymax
    @pos.y + (@h - @ky)/@scale
  end
  
  # Get bounds of the canvas. (Only valid when angle = 0)
  def bounds
    [xmin, ymin, xmax, ymax]
  end
  
  # Set the viewport to this canvas.
  def set
    GL.MatrixMode(GL::PROJECTION)
    GL.LoadIdentity
    GLU.Ortho2D(0, @w, 0, @h)
    GL.Translate(@kx, @ky, 0)
    GL.Rotate(@angle, 0,0,1)
    GL.Scale(@scale, @scale, 1)
    GL.Translate(-@pos.x, -@pos.y, 0)
    GL.MatrixMode(GL::MODELVIEW)
  end
  
  # Transform the mouse to world coordinates
  def mouse
    m = vec2($mx.quo(AS::SCREEN_W)*@w, (1.0 - $my.quo(AS::SCREEN_H))*@h)
    rot = @angle.deg2unit_vect
    return (m - vec2(@kx, @ky)).unrotate(rot)/@scale + @pos
  end
  
  # Set the bounds of the viewport.  Convenience method, to set a
  # simple viewport without createing a Canvas object.
  def self.set_bounds(left, bottom, right, top)
    GL.MatrixMode(GL::PROJECTION)
    GL.LoadIdentity
    GLU.Ortho2D(left, right, bottom, top)
    GL.MatrixMode(GL::MODELVIEW)
  end
end

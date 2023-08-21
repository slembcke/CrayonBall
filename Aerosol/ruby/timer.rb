# TickTimers keep a constant amount of time between ticks, calling a
# block of code if there is time before the tick ends.
class AS::TickTimer
  # The current tick count.
  attr_reader :ticks
  
  # The timestep for each tick. 1.0/(ticks per second)
  attr_reader :step
  
  # False if the timer is behind this tick.
  attr_reader :on_time
  
  # Create a timer object with the given ticks per second and maximum
  # frameskip.
  def initialize(tps, frameskip = 2)
    @ref = Time.now.to_f
    @step = 1.0/tps
    
    @ticks = 0
    @ref_ticks = 0
    
    @frameskip = frameskip
    @skipped = 0
  end
  
  def seconds
    @ticks*@step
  end
  
  # Returns a float value for the end of the tick.  Can be compared
  # against <tt>Time.now.to_f()</tt>.
  def tick_end_time
    @ref + (@ref_ticks + 1)*@step
  end
  
  # Delay until the end of the tick.  Returns False if the timer has
  # fallen behind.
  def sleep
    @ticks += 1
    
    time = Time.now.to_f
    remain = tick_end_time - time
    
    if remain > -@step
      @skipped = 0
      Kernel.sleep(remain) if remain > 0.0
      @ref_ticks += 1
      return true
    else
      if (@skipped += 1) > @frameskip
        @ref = time
        @ref_ticks = 0
        @frameskip = 0
        return true
      else
        @ref_ticks += 1
        return false
      end
    end
  end
end

# Framerate counter.
class AS::FPSCounter
  # Framerate statistics.
  attr_reader :min, :current, :max
  attr_reader :ticks, :to_s
  
  # Create a new framerate counter.  Samples is the number of ticks
  # between statistics updates.
  def initialize(samples)
    @samples = samples
    @min = 1_000_000.0
    @current = 0.0
    @max = 0.0
    @ticks = 0
  end
  
  # Advance the counter. Returns true when the statistics are updated.
  def tick
    if @ticks == @samples
      @ticks = 0
      @current = @samples/(Time.now.to_f - @ref)
      @min = [@current, @min].min
      @max = [@current, @max].max
      @to_s = sprintf("%.2f %.2f %.2f\n", min, current, max)
      return true
    end
    
    @ref = Time.now.to_f if @ticks == 0
    @ticks += 1
    
    return false
  end
end

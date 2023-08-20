
class MusicPlayer < AS::Music

  def play
    @isPlaying = true
    super
  end

  def toggle
    if @isPlaying
      pause
    else
      resume
    end
  end

  def pause
    @isPlaying = false
    super
  end

  def resume
    @isPlaying = true
    super
  end

  def stop
    @isPlaying = false
    super
  end

end

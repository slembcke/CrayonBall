AS::SoundStruct = Struct.new(:data, :rate, :format)

# Simple WAV file loader.  *NOTE:* Due to the simplicity of it's
# implementation, it may raise an execption on a WAV with extra tags.
# Run the WAV file through an editor to remove the extraneous tags.
class AS::WavFile < AS::SoundStruct
  Error = Class.new(Exception)
  
  Formats = {
	[1,  8] => AL::FORMAT_MONO8,
	[1, 16] => AL::FORMAT_MONO16,
	[2,  8] => AL::FORMAT_STEREO8,
	[2, 16] => AL::FORMAT_STEREO16,
  }
  
  #Read in a WAV file from _filename_
  def initialize(filename)
    open(filename) do|f|
      raise "RIFF tag not found in #{filename}" unless f.read(4)=='RIFF'
      size = f.read(4).unpack('V')[0]
      
      raise "WAVE tag not found in #{filename}" unless f.read(4)=='WAVE'
      raise "Format tag not found in #{filename}" unless f.read(4)=='fmt '
      
      tableSize = f.read(4).unpack('V')[0]
      raise "Bad table size in #{filename}: found #{tableSize} expected 16" unless tableSize==16
      raise "Unknown or compressed format for #{filename}" unless f.read(2).unpack('v')[0]==1
      
      channels = f.read(2).unpack('v')[0]
      self.rate = f.read(4).unpack('V')[0]
      #f.read(6)
      f.seek(6, IO::SEEK_CUR)
      bps = f.read(2).unpack('v')[0]
	  self.format = Formats[[channels, bps]]
      
      raise "Data tag not found in #{filename}" unless f.read(4)=='data'
      bytes = f.read(4).unpack('V')[0]
      self.data = f.read(bytes)
      
      self.data = self.data.unpack("v*").pack("S*") if bps==16 and RUBY_PLATFORM =~ /powerpc/
    end
  end
end

# Create an OpenAL buffer from a wav/ogg file.  Consider this class a
# temporary solution.  *NOTE:* May raise exceptions on WAV files with
# extraneous tags. See AS::WavFile
class AS::Sound
  attr_reader :buff
  
  def initialize(filename)
    sound = case extension = File.extname(filename).downcase
	when '.wav': AS::WavFile.new(filename)
	when '.ogg': AS.load_vorbis(filename)
	else raise "Unknown extension: #{extension}"
	end
	
    @buff = AL.GenBuffers(1)[0]
    AL.BufferData(@buff, sound.format, sound.data, sound.data.length, sound.rate)
  end
end

# Create an audio source from a AS::Sound.  Consider this class a
# temporary solution.  The number of SoundPlayers is limited to the
# number of sources OpenAL will provide. (which is indeterminate)
class AS::SoundPlayer
  # Takes a AS::Sound, and can optionally be looped.
  def initialize(sound, loop = false)
    @source = AL.GenSources(1)[0]
	
    AL.Sourcei(@source, AL::LOOPING, AL::TRUE) if loop
    AL.Sourcei(@source, AL::BUFFER, sound.buff);
  end

  # Play this sound.
  def play
    AL.SourcePlay(@source)
  end
  
  # Stop playing this sound
  def stop
    AL.SourceStop(@source)
  end
  
  # Set the volume of the source.
  def volume=(v)
    AL.Sourcef(@source, AL::GAIN, v)
  end
end

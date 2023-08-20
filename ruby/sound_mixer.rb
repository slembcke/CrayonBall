class SoundMixer
	class Sound
		attr_reader :buff
		
		def initialize(system, filename)
			@system = system
			sound = AS.load_vorbis(filename)
			
			@buff = AL.GenBuffers(1)[0]
			AL.BufferData(@buff, sound.format, sound.data, sound.data.length, sound.rate)
			
			ObjectSpace.define_finalizer(self, Sound.finalizer_block(@buff))
		end
		
		def play(pitch=1.0)
			@system.play(self, pitch)
		end
		
		def self.finalizer_block(id)
			lambda{AL.DeleteSources([id])}
		end
	end
	
	attr_reader :volume
	
	def initialize(channels)
		@channels = AL.GenSources(channels)
		@volume = 1.0
	end
	
	def load_sound(*args)
		Sound.new(self, *args)
	end
	
	def volume=(vol)
		@channels.each{|source| AL.Sourcef(source, AL::GAIN, vol)}
	end
	
	def set_vol_with_preview(vol)
		self.volume = vol
		AS::ResourceCache.load('sfx/lightning.ogg').play
	end
	
	def play(sound, pitch=1.0)
		source = @channels.find{|channel| AL.GetSourcei(channel, AL::SOURCE_STATE) != AL::PLAYING}
		return unless source
		
		AL.Sourcei(source, AL::BUFFER, sound.buff)
		AL.Sourcef(source, AL::PITCH, pitch)
		AL.SourcePlay(source)
	end
end

Mixer = SoundMixer.new(10)

module SoundLoader
	AS::ResourceCache::Loaders << self
	
	def self.match?(key)
		/.*\.ogg/ === key
	end
	
	def self.loader
		lambda{|key| Mixer.load_sound(key)}
	end
end


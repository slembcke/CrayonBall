
class Badge

	attr_reader :count, :text

	BadgeClasses = []

	def self.inherited(subclass)
		BadgeClasses << subclass unless subclass.to_s == 'ComboBadge'
	end
	
	def self.get_badges(gamestate)
		BadgeClasses.map{|klass| klass.new(gamestate) }
	end

	def initialize(gamestate)
	  @gamestate = gamestate
		@count = 0
	end

	def text
		self.class.display_name
	end
	
	def ball_clicked(ball)
	
	end
	
	def group_cleared(score, group_score, multiplier)
	
	end
	
	def end_game(gamestate)
		
	end
	
	def game_state_changed(gamestate)
		
	end
	
	def earn_badge
		@gamestate.board.add_notification(self.class.display_name)
		
		if self.class.respond_to?(:earn_sound)
			AS::ResourceCache.load(self.class.earn_sound).play
		end
		
		Stats[self.class.name.to_sym]+=1
		@count += 1
  end
	
	def marshal_dump
		@count
	end
	
	def marshal_load(loaded)
		@count = loaded
	end
	
end

class ScoreAchievementBadge < Badge

	def initialize(gamestate)
		super(gamestate)
	end
	
	def self.display_name; "Over 250k!"; end
	def self.texture_name; 'birthdayCake.png'; end
	
	def group_cleared(score, group_score, multiplier)
		if score > 250_000 * (@count + 1)
			earn_badge
		end
	end

end

class SurvivalBadge < Badge

	def self.texture_name; 'survivor.png'; end
	def self.display_name; "Survivor!"; end

	MinutesToSurvive = 3
	
	def initialize(gamestate)
		super(gamestate)
	end
	
	def game_state_changed(gamestate)
		if gamestate.ticks > 60*60*MinutesToSurvive * (@count + 1)
      earn_badge
		end
	end

end

class ComboBadge < Badge

	def initialize(gamestate)
		super(gamestate)
		@combo_reset = true

	  @combo_size = combo_size
	end 
	
	def group_cleared(score, group_score, multiplier)
		@combo_reset = true if multiplier < @combo_size

		if multiplier > @combo_size and @combo_reset
			earn_badge
			@combo_reset = false
		end
	end

end

class MegaComboBadge < ComboBadge
	def self.display_name; "Sparrow Combo!"; end
  def combo_size; 6; end
	def self.texture_name; 'smallCombo.png'; end
	#def self.earn_sound; 'sfx/sparrow.ogg'; end

end

class UltraComboBadge < ComboBadge
	def self.display_name; "Owl Combo!"; end
  def combo_size; 12; end
	def self.texture_name; 'mediumCombo.png'; end
	#def self.earn_sound; 'sfx/owl.ogg'; end

end

class GigaComboBadge < ComboBadge
	def self.display_name; "Eagle Combo!"; end
  def combo_size; 18; end
	def self.texture_name; 'largeCombo.png'; end
	#def self.earn_sound; 'sfx/eagle.ogg'; end

end

class EmptyBadge < Badge

	def self.texture_name; 'cleanSweep.png'; end
	def self.display_name; "Clean Sweep!"; end

	def game_state_changed(gamestate)
		if gamestate.ball_count < 10 and @count < 1 and gamestate.ticks > 60*3
			earn_badge
		end
	end
	
end

class NoSpecialsBadge < Badge

	def self.texture_name; 'noSpecials.png'; end
	def self.display_name; "No Specials!"; end
	
	def initialize(gamestate)
		super(gamestate)
		@lost_it = false
	end

	def ball_clicked(ball)
		@lost_it = true if ball.class != NormalBall
	end
	
	def end_game(gamestate)
		return if gamestate.mode == :drop || gamestate.ticks < 60*5
		earn_badge if @lost_it == false
	end

end

class TriggerFingerBadge < Badge

	def self.texture_name; 'triggerFinger.png'; end
	def self.display_name; "Trigger Finger!"; end

	def ball_clicked(ball)
		earn_badge if ball.body.vel.length > 40 and @gamestate.ticks > 60*3 and !(@gamestate === GrabGameStateDelegate)
	end

end

class ComebackBadge < Badge

	def self.texture_name; 'comebackKing.png'; end
	def self.display_name; "Comeback King!"; end
	#def self.earn_sound; 'sfx/brains.ogg'; end

	def initialize(gamestate)
		super(gamestate)
		@in_danger = false
	end

	def game_state_changed(gamestate)
		if gamestate.ball_count >= 105 and gamestate.ticks > 60*3
			@in_danger = true
		end
		
		if @in_danger && gamestate.ball_count < 45
			earn_badge
			@in_danger = false
		end
	end

end
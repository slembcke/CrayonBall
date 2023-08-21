# The score class tracks the score for a game, and also handles
# logic behind the combos, lifetime of a combo, multipliers,
# and the badge system.
class Score
	ResetTicks = 90

	attr_reader :score, :multiplier
	attr_reader :score_string, :combo_string
	attr_reader :badges

	def initialize(tick_source, gamestate)
		@tick_source = tick_source
		
		@ticks_of_last = -ResetTicks
		
		@multiplier = 1
		@score = 0
		@combo_score = 0
		
		@score_string = ""
		@combo_string = ""
		
		@badges = Badge.get_badges(gamestate)
		
		update_strings
	end
	
	def combo_score
		combo? ? @combo_score : 0
	end

	def combo_multiplier
		combo? ? @multiplier - 1 : 0
	end

	# A group of balls have been cleared, either by matching, or 
	# they were all hit by a bomb, etc.
	def group_cleared(num)
		reset_combo unless combo?
		
		score = 100*num
		group_score = score*@multiplier
		
		@score += group_score
		@combo_score += group_score
		
		@multiplier += 1
		@ticks_of_last = @tick_source.ticks

		@badges.each{|badge| badge.group_cleared(@score, group_score, @multiplier)}	
		Stats[:longestCombo] = @multiplier if @multiplier > Stats[:longestCombo] 
		Stats[:biggestCombo] = @combo_score if @combo_score > Stats[:biggestCombo] 

		return group_score
	end
	
	def ball_clicked(ball)
		@badges.each{|badge| badge.ball_clicked(ball)}
	end

	#Triggered when the game ends, passed a GameState for inspection.
	def end_game(gamestate)
		@badges.each{|badge| badge.end_game(gamestate)}	
	end
	
	# Triggered when something in the gamestate changed and we might want 
	# to inspect it for badge awarding.
	def game_state_changed(gamestate)
		@badges.each{|badge| badge.game_state_changed(gamestate)}	
	end
	
	def badges_earned
		@badges.select{|badge| badge.count > 0}
	end
	
	SCORE_FORMAT = "Score: %s"
	COMBO_FORMAT = "Combo: %s (%dx)"
	
	def update_strings
		@score_string.replace(sprintf(SCORE_FORMAT, Misc.commas(@score)))
		@combo_string.replace(sprintf(COMBO_FORMAT, Misc.commas(combo_score), combo_multiplier))
	end
	
	# Combo is still open to add new chained scores
	def combo?
		@tick_source.ticks - @ticks_of_last < ResetTicks
	end
	
private
	def reset_combo
		@multiplier = 1 
		@combo_score = 0
	end
end

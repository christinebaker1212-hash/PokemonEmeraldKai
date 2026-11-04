@ Moemon Maker: what the people the user adds do (src/maker/maker_overrides.c, docs/MOEMON_MAKER.md)

@ An item ball the user put down: the item is the ball's own (Maker_ItemBallItem reads it from the person), taken once
@ (the pickup removes the ball, setting its flag: a Maker flag of its own, src/maker/maker_flags.c)
Maker_EventScript_ItemBall::
	special Maker_ItemBallItem
	callstd STD_FIND_ITEM
	end

@ A person the user put down who has nothing to say yet
Maker_EventScript_Nothing::
	end

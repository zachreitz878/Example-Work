extends RigidBody2D

var is_clicked = false #mouse or touchscreen input
var speed = 2.5
var mouse_dir = Vector2()
var sounds
var thruster_sound
var star_sound
var is_dead = false
var power = false
var fuel = 100
var fuel_default_rate = 2.5
var fuel_thrust_rate = 4
var star_fuel = 5
var stars_collected = 0
var distance = 0

func _ready():
	self.set_pos(Vector2(get_viewport_rect().size.width/2, get_viewport_rect().size.height*0.75))
	self.look_at(Vector2(get_viewport_rect().size.width/2, 0))
	set_process_input(true)
	set_fixed_process(true)
	sounds = get_node("../Game/SamplePlayer")
	#Grab thruster sound at beginning to manipulate in loop
	sounds.set_default_volume(0.0)
	thruster_sound = sounds.play("thruster")
	sounds.set_volume(thruster_sound, 0)
	sounds.set_default_volume(3)

#for physics stuff
func _integrate_forces(s):
	if(is_dead == false):
		if(is_clicked == true):
			mouse_dir = (get_viewport().get_mouse_pos() - get_pos()).normalized() #vector from rocket to mouse
			var dir = s.get_linear_velocity() + mouse_dir * speed
			s.set_linear_velocity(dir)
	else:
		s.set_linear_velocity(Vector2(0,0))

func _fixed_process(delta):
		
	if(is_dead && get_node("Sprite").is_visible()):
		sounds.stop_all()
		sounds.play("laser")
		sounds.play("rumble")
		get_node("Explosion/anim").play("explosion_anim")
		get_node("Physics Body").queue_free()
		get_node("Sprite").hide()
		get_node("Flames").set_emitting(false)
		get_node("../Game/dead_time").start()
	else:
		distance += delta
		
	if(get_node("Fuel_Timer").get_time_left() != 0 && get_node("Fuel_Timer").get_time_left() < 0.1):
		is_dead = true
	
	#Check collisions
	if(get_colliding_bodies().size() > 0 && is_dead == false):
		for thing in get_colliding_bodies():
			if (thing.is_in_group("collectable")):
				thing.queue_free()
				var voice = sounds.play("pickup")
				stars_collected += 1
				#Set limit on how much fuel you can get to
				if(fuel + star_fuel < 100):
					fuel += star_fuel
					get_node("../").score += 1 
				else:
					fuel = 100
					get_node("../").score += 2
					
			if (thing.is_in_group("powerup")):
				thing.queue_free()
				sounds.play("ufo_collect")
				
				if(fuel + star_fuel < 100):
					fuel += star_fuel
					get_node("../").score += 10
				else:
					fuel = 100
					get_node("../").score += 15
					
				power = true
				get_node("Power_Timer").start()
				
			#if you hit an asteroid, you die
			if (thing.is_in_group("asteroid") && power == false):
				is_dead = true
	
	#UFO powerup stuff
	if(get_node("Power_Timer").get_time_left() < 0.2 && get_node("Power_Timer").get_time_left() != 0):
		power = false
	if(power == true):
		get_node("Sprite").set_modulate(Color(0,1,0,1))
		set_angular_damp(1)
		set_friction(1)
		set_bounce(0)
		#get_node("Flames").set_randomness(
	else:
		get_node("Sprite").set_modulate(Color(1,1,1,1))
		set_angular_damp(0)
		set_friction(0)
		set_bounce(1)
	
	
	if(is_clicked == true && is_dead == false):
		#Orient Rocket to face screen touched position
		look_at(get_viewport().get_mouse_pos())
		#Emit flames
		get_node("Flames").set_emitting(true)
		if(fuel - delta * fuel_thrust_rate > 0.000):
			if(power == false): #don't lose fuel while power active
				fuel -= delta * fuel_thrust_rate
		else:
			fuel = 0.000
		
		#gradually increase thruster volume
		if(sounds.get_volume(thruster_sound) < 2.5):
			sounds.set_volume(thruster_sound, sounds.get_volume(thruster_sound) + delta *9)
	#natural fuel degredation
	else:
		if(fuel - delta / fuel_default_rate > 0.0000):
			if(power == false):
				fuel -= delta / fuel_default_rate
		else:
			fuel = 0.0000
	
		#gradually decrease thruster volume if not clicked
		if(sounds.get_volume(thruster_sound) > 0):
			sounds.set_volume(thruster_sound, sounds.get_volume(thruster_sound) - delta *9)
			
		get_node("Flames").set_emitting(false)
		
	
			
	if(fuel == 0):
		is_clicked = false
		if(get_node("Fuel_Timer").get_time_left() == 0):
			get_node("Fuel_Timer").start()
	else:
		get_node("Fuel_Timer").stop()

func _input(event):
	if (event.type==InputEvent.MOUSE_BUTTON || event.type==InputEvent.SCREEN_TOUCH):
		if(event.is_pressed()):
			is_clicked = true
		elif(is_clicked):
			is_clicked = false
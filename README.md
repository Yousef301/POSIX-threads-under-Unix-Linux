# Ant Searching for Food Simulation
We would like to build a multi-threading application that simulates the behavior of a group
of ants once any of them smell the presence of food. Keep in mind that ants behavior
might be more complex than the simulation we’re building. We intend to keep things
simple for now. The simulation can be described as follows:

    • A user-defined number of ants are created when the simulation starts (e.g. 100 ants).
      These ants are randomly located on the screen and walk in random directions and
      random speeds. Assume the direction of walking can be North (N), South (S), East
      (E), West (W), North-East (NE), North-West (NW), South-East (SE), South-West
      (SW). The speed is an integer value belonging to the range [1. . . 10].
      When an ant hits the limit of the simulation window, it continues walking with the
      same speed but with an additional 45◦ angle, either CW or CCW (random). The
      behavior continues forever.
      
    • Pieces of food will be placed in random locations every user-defined amount of time.
      Assume an ant can smell the presence of food if the food is placed anywhere within a
      user-defined distance from the ant. Once it smells the presence of food, it releases a
      pheromone which is a chemical substance that triggers a social response in the ants
      next to it. Assume the following:
      
        – All ants whose distance with the ant that smelled the food is below a user-
          defined distance will shift direction and head towards the food. In addition,
          these ants will release a pheromone that is less powerful but will help in prop-
          agating the social response in more ants (the released pheromone amount is
          indirectly proportional to the distance to the food). However, as these ants get
          closer to the food source, they will release bigger quantities of pheromone.
       
        – Ants that smell pheromone but with lesser quantities will change direction by
          an angle of 5◦ in the direction of the food position per second. If the pheromone
          smelled by an ant drops below a user-defined quantity, it continues in the current
          direction of movement.
        
        – If an ant gets pheromone smell from 2 different food locations, it will favor the
          one with the higher smell.
          
    • When ants are gathered on top of the piece of food, they will stop walking and will
      each eat a small portion of that food until it is all gone. Of course food eating should
      be monitored so 2 ants do not eat the same food portion. Assume each ant can eat
      a user-defined portion (in percent) of the food per second.
      
    • The simulation ends when a user-defined amount of time (in minutes) has elapsed.

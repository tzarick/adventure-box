# adventure-box

A box that only opens / unlocks when it arrives at specific geographic coordinates.

## Disclaimer

If you are one of the few people I gave one of these boxes to and you still haven't reached your destination and you somehow found this... 1) get a move on with it smh, and 2) leave now!!! Be gone! Mild spoilers live here.

## Background

This project was made as a Christmas gift for each member (each couple, technically) of my family: a lockbox that is locked when they unwrap it and only openable once they take it with them to some specific location on the globe. I thought it would be a fun reverse geocache / scavenger hunt.

I like the idea of "opening" the box as an adventure / experience rather than seeing it as just a thing in a box that requires some extra work to open.

## Functionality

The small display on the outside of the box shows:

1. the **distance** in miles away from the destination, and
2. the **heading** in degrees that you need to proceed in

This specific program currently takes in 2 sets of coordinates:

1. a final destination, and
2. a checkpoint

Initially, the checkpoint is treated as the destination (the user should not know this, as all they see is a distance and heading, and know that they need to take the box somewhere). At the checkpoint, there was a physical clue irl which eventually prompted them to interact with the box in a way that passes the checkpoint and reveals their new distance and heading (to the final destination).

Upon arrival at the final destination, the 3-digit box combination is displayed on-screen.

## Hardware / Parts

Main box components:

- 3-digit combination aluminum lockbox
- 128 x 64 OLED display
- NEO-6M GPS module + antenna
- Arduino Pro Mini 5V
- Piezo buzzer
- Push button
- Power switch
- Small 5V/1A power bank
- USB charging cable

I'll include a full list of the specific parts I used at a later time (TODO).

## Software

I consulted some examples / projects from the NEO-6M GPS module and projects that involve both GPS module and small I2C OLED display. I also found some specific piezo buzzer note encodings and a short sequence or two online. Will include specific references at a later time (TODO).

Am going to clean up the code a bit, hopefully soon (TODO).

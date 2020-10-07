# FicsItCam [![Build Status](https://jenkins.massivebytes.net/job/FicsIt-Cam/job/master/badge/icon)](https://jenkins.massivebytes.net/job/FicsIt-Cam/job/master/)

This Satisfactory mod is mainly intended for Content Creators to allow the creation of beautiful camera animations.

The mod introduces a couple of new chat commands:
- `/fic create <animation>`
 Creates a new animation with the given name.
- `/fic delete <animation>`
 Deletes the animation with the given name.
- `/fic play <animation>`
 Plays the animation with the given name.
- `/fic edit <animation>`
 Opens the animation editor for the animation with the given name.

Animation Editor:
- The animation editor allows for creating and editing animation.
- On the bottom you can find the Timeline scrubber and range selector which allow you to view and change the currently active frame in the animation.
- In the lower-left corner you find the frame settings were you can change the the active frame, change the animation range (length) and the selected frame range.
- On the left side you can find the details panel were you can view and change the current values of the properties (like location, rotation and FOV)
- Every property has a keyframe control which allows to set the keyframe at the active frame to the current value when clicking on the control and there is no keyframe or the keyframe differs from the current value.
- If at the current frame is a keyframe and the keyframe-value doesn't differ from the current value (has not changed) then a click removes the keyframe.
- If there is no keyframe is set at the active frame, the control is grey (nowere in the animation is a keyframe for that property) or dark orange (somewere else in the animation is a keyframe for that property).
- If there is a keyframe at the active frame, the control is orange (when the keyframe has not changed) or purple/blue (when the keyframe has changed)
- A double click on the keyframe control results in deletion of all keyframes of that property
- When clicking on the viewport you change into movement mode.
- When the active frame changes, all properties get updated to the value in the animation at that time. If no keyframe is set, is just stores that value.
- R-Clicking on a keyframe control allows to changed the interpolation type of the keyframe

You can also use following key inputs:
- `Right Alt` -
 Allows to switch between movement mode and cursor mode.
- `W`, `A`, `S`, `D`, `Space`, `Left Alt` and `Mouse Movement` -
 Allows to move the camera around while beeing in movement mode
 When holding shift you can also move faster around
- `Ctrl` + `Mouse-Wheel` -
 Moves the active frame around
- `Shift` + `Mouse-Wheel` -
 Changes the camera movement speed
- `N` -
 Jump to the previous keyframe
- `M` -
 Jump to the next keyframe
- `Arrow Left` -
 Moves the active frame to the left
- `Arrow Right` -
 Moves the active frame to the right
- `I` -
 Sets the keyframes of all properties at the active frame to the current values.
 If all properties have at the active frame unchanged keyframes, removes all keyframes at the current frame.

## Contributors
- Panakotta00 (Development)
- Deantendo (Icon)

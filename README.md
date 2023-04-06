# FicsItCam [![Build Status](https://jenkins.massivebytes.net/job/FicsIt-Cam/job/master/badge/icon)](https://jenkins.massivebytes.net/job/FicsIt-Cam/job/master/)

This Satisfactory mod is mainly intended for Content Creators to allow the creation of beautiful camera animations, timelapse recordings and other camera related things.

Visit [SMR (aka. ficsit.app)](https://ficsit.app/mod/FicsItCam) for more information.

# Third-Party
This mod uses a third party library called [FFmpeg](https://ffmpeg.org) for converting images taken with FicsIt-Cam to MP4 files.

# Guide/Tutorial
Currently the best way to get to know all the features of FicsIt-Cam is [this in-depth Guide](https://www.youtube.com/watch?v=6Gl5rOFyqmM).

## Quick Help
Animation Rendering:
Animations can be render into a mp4 (without audio). These mp4 files are located at `%localappdata%\FactoryGame\Saved\SaveGames\FicsItCam\<Scene Name>`.
The video should have the resolution set in the scene and as name the timestamp at start of the recording.

Timelapse Cameras:
Timelapse cameras can be used to take images of you factory in prediodic intervals and store these images in your filesystem.
Your gameplay wont bit disrupted, tho depending on your hardware you might experience a small lag when the image gets taken.
Images are named with the timestamp of start of the timelapse (per game save session) and a incrementing number and are located under `%localappdata%\FactoryGame\Saved\SaveGames\FicsItCam\<Timelapse Camera Name>`.
Most Video Editing software does recognize the image sequences automatically on import, but sometimes you may need to use a tool like FFmpeg to convert them to a video with your specefied settings like framerate.

## Written Documentation
You can find a written documentation of the modification on [SMR aka. ficsit.app](https://ficsit.app/mod/FicsItCam).

## Contributors
- Panakotta00 (Development)
- Deantendo (Icon)

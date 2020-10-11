@REM run not from batch required to remove %% symbol! 
@REM 	ffmpeg -y -loop 1 -i img_white.png -c:v libx264 -t 15 -pix_fmt yuv420p -vf scale=320:240 -vf "drawtext=text='%{n}'" tst_1.mp4

ffmpeg -y -loop 1 -i img_white.png -c:v libx264 -t 15 -pix_fmt yuv420p -s 320x240 -vf "drawtext=text='%%{n}'" tst_1.mp4
@REM ffmpeg -y -loop 1 -i img_white.png -c:v libx265 -t 15 -pix_fmt yuv420p -s 320x240 -vf "drawtext=text='%%{n}'" tst_1.mp4
@REM ffplay tst_1.mp4


@REM ffmpeg -y -loop 1 -i img_white.png -c:v libx265 -t 15 -pix_fmt yuv420p -s 320x240 -vf "drawtext=text='%%{n}'" tst_265.mp4
ffmpeg -y -loop 1 -i img_white.png -r 100 -c:v libx264 -t 5 -pix_fmt yuv420p -s 320x240 -vf "drawtext=text='%%{n}'" tst_r100_264.mp4
ffmpeg -y -loop 1 -i img_white.png -r 1 -c:v libx264 -t 15 -pix_fmt yuv420p -s 320x240 tst_r1_264.mp4
ffmpeg -y -i tst_r1_264.mp4 -vf "drawtext=text='%%{n}'" tst_r1_264_txt.mp4

@REM ffplay tst_r100_264.mp4


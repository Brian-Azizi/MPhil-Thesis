% settings
height = 128;
width = 128;
frames = 64;

input = 'foreman-128-128-64';
blockDim = '8-8-8';
corrPerc = '70%';
corrMode = 'timeRays';
scale = 2;

frameRate = 20;

origFile = strcat('../',input,'.txt');
corruptFile = strcat('../Simulations/',input,'/',blockDim,...
    '_',corrPerc,'_',corrMode,'_',num2str(scale),...
    '_corrupted_',input,'.txt');
recoverFile = strcat('../Simulations/',input,'/',blockDim,...
    '_',corrPerc,'_',corrMode,'_',num2str(scale),...
    '_recovered_',input,'.txt');


original = txt2rawBW(origFile, height, width, frames);
corrupt = txt2rawBW(corruptFile, height, width, frames);
recover = txt2rawBW(recoverFile, height, width, frames);

implay(uint8(corrupt), frameRate);
implay(uint8(recover), frameRate);
implay(uint8(original), frameRate);
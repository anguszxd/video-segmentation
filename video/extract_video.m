clc;
close all;

% Open an sample avi file

filename = 'save1.avi';
mov = MMREADER(filename);

% Output folder

outputFolder = fullfile(cd, 'frames');
if ~exist(outputFolder, 'dir')
    mkdir(outputFolder);
end

%getting no of frames

numberOfFrames = mov.NumberOfFrames;
numberOfFramesWritten = 0;
for frame = 1 : numberOfFrames    
    thisFrame = read(mov, frame);
    outputBaseFileName = sprintf('%3.3d.png', frame);
    outputFullFileName = fullfile(outputFolder, outputBaseFileName);
    imwrite(thisFrame, outputFullFileName, 'png');
    progressIndication = sprintf('Wrote frame %4d of %d.', frame,numberOfFrames);
    disp(progressIndication);
    numberOfFramesWritten = numberOfFramesWritten + 1;
end
progressIndication = sprintf('Wrote %d frames to folder "%s"',numberOfFramesWritten, outputFolder);
disp(progressIndication);
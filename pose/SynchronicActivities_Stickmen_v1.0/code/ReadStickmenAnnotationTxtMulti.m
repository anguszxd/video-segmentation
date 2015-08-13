function [lF] = ReadStickmenAnnotationTxtMulti(txtfile,optionalfieldname,defaultvalue)
% lF = ReadStickmenAnnotationTxt(txtfile)
% Read annotations in file 'txtfile' and stores them in a struct-array.
% 
% Input:
%  - txtfile containing annotations:
%
% Output:
%  - lF: struct-array with fields:
%      .stickmen.coor: matrix [4, nparts]. lF(k).coor(:,i) --> (x1, y1, x2, y2)' 
%      .optionalfieldname = defaultvalue;
%      if annot file contains frame numbers then .frame field exist of type double (containing a frame number)
%      if annot file contains filenames then .filename field exist of type string (containing an image filename)
%
%
% !!!! supports multiple annotations per frame
% Input file format:
% <file_name_i or frame_nr_i> [#annotations #parts_per_annotation] 
%  <x11>  <y11> <x12> <y12>
%  <x21>  <y21> <x22> <y22>
%  <x31>  <y31> <x32> <y32>
%  <x41>  <y41> <x42> <y42>
%  <x51>  <y51> <x52> <y52>
%  <x61>  <y61> <x62> <y62>  
%
% if #annotations not given then assumes 1
% if #parts_per_annotation not given then assumes 6
%
%
%
% See also DrawStickmen
%
% by Eichner/2009
%

% Open file
fid = fopen(txtfile, 'rt');
if fid < 1,
   error([' Can not open file ', txtfile]);   
end




% Read frames and annotations
nread = 0;
stop = false;
while ~stop,
  [line] = fgetl(fid); % Read element
  if isequal(line,-1)
    stop = true; 
  else % read annotation
    nread = nread+1;
    % first line of the annotation contains filename or frame number 
    % [optionally number of annotated stickmen and number of body parts annotated per stickman]
    temp = textscan(line,'%s');
    columns = temp{1}; %select first format's element matches 
    
    % assure backward compatibility
    NANNOTS = 1;
    PARTS = 6;
    if length(columns) > 1
      NANNOTS = str2double(columns{2});
    end
    if length(columns) > 2
      PARTS = str2double(columns{3});
    end
    if isempty(regexp(columns{1},'[a-zA-Z]','once'))
      % frame number
      lF(nread).frame = str2double(columns{1}); 
    else
      % filename

      lF(nread).filename = columns{1};
    end
    
    for an = 1:NANNOTS
      for k = 1:PARTS,
        [line] = fgetl(fid); % Read element
        temp = textscan(line,'%f',4);
        coor = temp{1};
        if size(coor,1) ~= 4
          error('incomplete annotation');
        else
          [lF(nread).stickmen(an).coor(1:4,k)] = coor; % Read coordinates for part
        end
      end % k
    end
    
    if nargin > 2
      eval(['lF(nread).' optionalfieldname '=' defaultvalue ';'])
    end
      
   end
end

% Close file
fclose(fid);
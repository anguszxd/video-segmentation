function bb = detBBFromStickman(stick,classname,params)
% function minBB(stick,classname)
% stick in format rows: [x1; y1; x2; y2] cols: nsticks
% returns bb in format [minx miny maxx maxy]
% so far just for ubf
% requires params.det_hwratio(classid)
stick = double(stick);
classid = class_name2id(classname);
switch classid
  case {2}
    head_center = (stick(1:2,6)+stick(3:4,6))/2;
    torso_center = (stick(1:2,1)+stick(3:4,1))/2; 
    stickmen_center = (head_center+torso_center)/2;
    miny = min([stick(2,6) stick(4,6) torso_center(2) ]); % min y of torso and head
    maxy = max([stick(2,6) stick(4,6) torso_center(2) ]); %max y of head and torso center
%       height = abs(miny-maxy);
%       width = height/params.det_hwratio(classid);
%       minx = stickmen_center(1) - width/2;
%       maxx = stickmen_center(1) + width/2; 

%     Alternative box generation used for small head sticks (face sticks) 
%     used in IJCV submission on ethz pascal stickmen dataset
%
    height= 1.2 * sqrt(max(sum((repmat(torso_center,1,2)-reshape(stick(:,6),2,2)).^2,1)));
    width = height/params.det_hwratio(classid);
    minx = stickmen_center(1) - width/2;
    maxx = stickmen_center(1) + width/2;
    miny = min(stick([2 4],6))-0.2*height;
    maxy =miny + height;

    bb = [minx miny maxx maxy];

  case 1 % profile upper body
    error('not written yet');
  case 3 % full body
    error('not written yet');
end

end
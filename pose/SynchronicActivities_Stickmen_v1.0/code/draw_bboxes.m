function draw_bboxes(imfig, bb, col, thick)
% Draw boxes on top of image
% imfig - image path, image or figure handle
% bb - set of bboxes where bb(i,:) = [minx miny maxx maxy]
% col - box color
% thick - box thickness 

if ishandle(imfig)
  figure(fig);
elseif ~isempty(imfig)
  imshow(imfig)
  axis equal;
  axis off;
end

if nargin < 3 || isempty(col)
  col = 'r';
end

if nargin < 4
  thick = 3;
end

line([bb(:,1) bb(:,1) bb(:,3) bb(:,3) bb(:,1)]', [bb(:,2) bb(:,4) bb(:,4) bb(:,2) bb(:,2)]', 'color', col, 'linewidth', thick);

drawnow;
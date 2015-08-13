function [boxes] = regressBoxes (boxin, par, multiTag)
% Resize "boxin" according the regression parameters
% "Boxin" must be a 2 dimensional matrix
% If "multitag" flag is passed, then the routine expects a n x 6 matrix,
% where each row has the format [x1 y1 x2 y2 score tag]. In this case the
% parameters "par" must have as many rows as different tags values

if nargin < 3 || isempty(multiTag)
  multiTag = false;
end

boxes = [];
par_ = par;

for i = 1 : size(boxin,1)

  oldx = boxin(i,1);
  oldy = boxin(i,2);
  oldw = boxin(i,3) - boxin(i,1);
  oldh = boxin(i,4) - boxin(i,2);

  if multiTag
    % If we use the multitag information then we set the correct regressor
    % for this tag
    par_ = par(boxin(i,6),:);
  end
  
  % Regress the dimension of the new box
  newx = oldx - round(oldw * par_(1));
  newy = oldy - round(oldh * par_(2));
  neww = oldw * par_(3);
  newh = neww * par_(4);

    
  if size(boxin,2) == 4
    boxes(i,:) = [newx newy newx+neww newy+newh]; 
  else
    boxes(i,:) = [newx newy newx+neww newy+newh boxin(i,5:end)]; % Also add other information
  end
end

end
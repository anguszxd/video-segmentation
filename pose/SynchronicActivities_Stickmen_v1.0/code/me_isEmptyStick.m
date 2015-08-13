function out = me_isEmptyStick(sticks)
  sizes = size(sticks);
  sticks = sticks(:,:); % reshape it to be coor x nsticks vector
  temp = sum(isnan(sticks),1);
  out = temp == size(sticks,1); % occluded when all coordinates are nan
  if(any(temp > 0 & temp < size(sticks,1))) % some coordinates are nan only!
    error('invalid sticks');
  end
  out = reshape(out,[1 sizes(2:end)]);
end
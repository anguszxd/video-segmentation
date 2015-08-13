function x = me_tostr(x)
if ischar(x)
  return
elseif iscell(x)
  x = cell2str(x);
elseif isnumeric(x)
  x = num2str(x);
end
  
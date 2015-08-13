function s = cell2str(c, sep)

% converts cell array of strings into a single long string;
% adds a separator sep between strings
%
% if sep not given
% -> sep = ' '
%

if nargin < 2
  sep = ' ';
end

for cix = 1:length(c)
  if cix == 1
    s = c{cix};
  else
    s = [s sep c{cix}];
  end
end

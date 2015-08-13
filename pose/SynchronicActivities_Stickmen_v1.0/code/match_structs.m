function [GT Test] = match_structs(GT,Test,exclude_fields)
% match GT and Test struct vectors according to all fields except exclude_fields 
% if Test entries not present in GT -> remove
% if GT entries not present in Test -> append with empty exclude_fields

if ischar(exclude_fields)
  exclude_fields = {exclude_fields};
end


sorting_fields = sort(fieldnames(rmfield(GT,exclude_fields)));

tmpGT = rmfield(GT,exclude_fields);
tmpTest = rmfield(Test,exclude_fields);

serialGT = cell(size(GT));
serialTest = cell(size(Test));

for i=1:numel(tmpGT) 
  serialGT{i} = '';
  for f=1:length(sorting_fields)
    serialGT{i} = [serialGT{i} sorting_fields{f} me_tostr(tmpGT(i).(sorting_fields{f}))];
  end
end

for i=1:numel(tmpTest)
  serialTest{i} = '';
  for f=1:length(sorting_fields)
    serialTest{i} = [serialTest{i} sorting_fields{f} me_tostr(tmpTest(i).(sorting_fields{f}))];
  end
end

[isect iGT iTest] = intersect(serialGT,serialTest);

Test = orderfields(Test(iTest)); % remove test entries not present in the ground truth


if numel(isect) == numel(serialGT)
  GT = orderfields(GT(iGT));
else
  iMiss = setdiff(1:length(GT),iGT);
  disp('WARNING: some ground-truth entries are missing in the testset -> appending empty entries to testset')
  
  addTest = tmpGT(iMiss);
  for f=1:length(exclude_fields)
    addTest(end).(exclude_fields{f}) = struct([]);
  end
  addTest = orderfields(addTest);
  Test(end+1:end+length(iMiss)) = addTest;
  
  addGT = orderfields(GT(iMiss));
  GT = GT(iGT);
  GT = orderfields(GT);
  GT(end+1:end+length(iMiss)) = addGT;
  
end
  






% 1. Load the data from CSV
opts = detectImportOptions('elevator_data.csv');
T = readtable('elevator_data.csv', opts);

% 2. Calculate means for each group
uniqueGroups = unique(T.Group_ID);
meanPressures = zeros(length(uniqueGroups), 1);

for i = 1:length(uniqueGroups)
    groupId = uniqueGroups(i);
    meanPressures(i) = mean(T.Pascal(T.Group_ID == groupId));
end

% 3. Plot Raw Samples
figure(1);
hold on;
for i = 1:length(uniqueGroups)
    gid = uniqueGroups(i);
    y = T.Pascal(T.Group_ID == gid);
    plot(y, 'LineWidth', 1.5, 'DisplayName', ['Floor Group ' num2str(gid)]);
end
xlabel('Sample Index (within group)');
ylabel('Pressure (Pascal)');
title('Pressure Readings per Floor Group');
legend('show');
grid on;

% 4. Plot Mean Steps
figure(2);
bar(uniqueGroups, meanPressures, 'FaceColor', [0 0.4470 0.7410]);
xlabel('Floor Group ID');
ylabel('Average Pressure (Pa)');
title('Mean Pressure per Floor');
% Zoom into the interesting range
ylim([min(meanPressures)-50, max(meanPressures)+50]);
grid on;
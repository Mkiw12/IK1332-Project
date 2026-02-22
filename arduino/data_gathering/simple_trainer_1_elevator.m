data = readtable('data_real_ele.csv', 'VariableNamingRule', 'preserve');
P = data.Pressure_Pa;

% to explain to myself. We're looking at 20 samples at a time. If pressure
% is moving too much, assume not floor. (dev). if pressure is flat, we mark
% it as a potential flloor. 
window_size = 20; 
p_std = movstd(P, window_size);
is_green = p_std < 1.1; % Only use the flattest parts of the data

%
% K-style clustering. looking for <x_floors>*2 <-- random number i chose
% idk tbh
%  amount of floors. k clustering
% should have some spare incase balls don't land properly
x_floors = 7;

[idx, potential_thetas] = kmeans(P(is_green), 2*x_floors, 'Replicates', 5);
potential_thetas = sort(potential_thetas, 'descend');

% if two floors close together merge em
unique_thetas = potential_thetas(1);
for i = 2:length(potential_thetas)
    % compare current potential floor to the last unique one we saved
    if abs(potential_thetas(i) - unique_thetas(end)) > 10.0
        unique_thetas = [unique_thetas; potential_thetas(i)];
    end
end

% diff between this floor and next floor
theta_f1 = unique_thetas(1);
offsets = unique_thetas - theta_f1;

% vis
figure;
plot(P, 'Color', [0.8 0.8 0.8]); hold on;


%make green where good data
plot(find(is_green), P(is_green), 'g.', 'MarkerSize', 5);

yline(unique_thetas, 'r-', 'Learned Floors', 'LineWidth', 2);

title(['Detected ', num2str(length(unique_thetas)), ' Unique Floors']);
grid on;

fprintf('--- UPDATED OFFSETS ---\n');
disp(offsets');

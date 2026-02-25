data = readtable('imutest2.csv', 'VariableNamingRule', 'preserve');
P = data.Pressure_Pa;

% Number of floors in your elevator
x_floors = 5;

% to explain to myself. We still look at variance to find still windows,
% but now we also use derivative + imu to verify movement.
window_size = 20;
p_std = movstd(P, window_size);
dup_tol = 10;

p_std = movstd(P, window_size);
dP = data.Pressure_dPaPerSec;

ax = data.AccelX_mg;
ay = data.AccelY_mg;
az = data.AccelZ_mg;
accel_mag = sqrt(ax.^2 + ay.^2 + az.^2);
accel_dev = abs(accel_mag - 1000);

% thresholds (simple percentile approach)
jitter_threshold = prctile(p_std(isfinite(p_std)), 70);
derivative_threshold = prctile(abs(dP(isfinite(dP))), 70);
accel_threshold = prctile(accel_dev(isfinite(accel_dev)), 70);

still_ratio = mean(p_std(isfinite(p_std)) < jitter_threshold);

fprintf('--- LEARNED THRESHOLDS (UNSUPERVISED) ---\n');
fprintf('JITTER_THRESHOLD = %.4f Pa\n', jitter_threshold);
fprintf('DERIVATIVE_THRESHOLD = %.4f Pa/s\n', derivative_threshold);
fprintf('ACCEL_DEVIATION_THRESHOLD = %.2f mg\n', accel_threshold);
fprintf('Still ratio below std threshold = %.3f\n', still_ratio);

% I cant be bothered doing gyro simplyfiing
moving_by_std = p_std > jitter_threshold;
moving_by_dp_and_acc = (abs(dP) > derivative_threshold) & (accel_dev > accel_threshold);
is_moving = moving_by_std | moving_by_dp_and_acc;

% still samples for floor clustering
is_green = ~is_moving;

if nnz(is_green) < 2 * x_floors
    error('Not enough still samples after filtering. Try a longer run.');
end

[~, potential_thetas] = kmeans(P(is_green), 2 * x_floors, 'Replicates', 5);
potential_thetas = sort(potential_thetas, 'descend');

% if two floors close together merge em
unique_thetas = potential_thetas(1);
for i = 2:length(potential_thetas)
    % compare current potential floor to the last unique one we saved
    if abs(potential_thetas(i) - unique_thetas(end)) > dup_tol
        unique_thetas = [unique_thetas; potential_thetas(i)]; %#ok<AGROW>
    end
end

% diff between this floor and next floor
theta_f1 = unique_thetas(1);
offsets = unique_thetas - theta_f1;

% plot 1: pressure + moving/still coloring
figure;
plot(P, 'Color', [0.8 0.8 0.8]); hold on;
plot(find(~is_moving), P(~is_moving), 'b.', 'MarkerSize', 5);
plot(find(is_moving), P(is_moving), 'r.', 'MarkerSize', 5);
yline(unique_thetas, 'r-', 'Learned Floors', 'LineWidth', 2);
title(['Detected ', num2str(length(unique_thetas)), ' Floors | Blue=Still Red=Moving']);
grid on;

% plot 2: threshold sanity checks
figure;
tiledlayout(3,1);

nexttile;
plot(p_std, 'b'); hold on;
yline(jitter_threshold, 'r--', 'std threshold');
title('Pressure std');
grid on;

nexttile;
plot(dP, 'm'); hold on;
yline(derivative_threshold, 'r--', 'dP threshold');
yline(-derivative_threshold, 'r--');
title('Pressure derivative (Pa/s)');
grid on;

nexttile;
plot(accel_dev, 'k'); hold on;
yline(accel_threshold, 'r--', 'acc threshold');
title('|AccelMag - 1000| (mg)');
grid on;

fprintf('--- UPDATED OFFSETS ---\n');
disp(offsets');

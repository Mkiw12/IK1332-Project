data = readtable('hemhiss.csv', 'VariableNamingRule', 'preserve');
P = data.Pressure_Pa;

% to explain to myself. We still look at variance to find still windows,
% but now we also use derivative + imu to verify movement.
window_size = 20;
p_std = movstd(P, window_size);
dup_tol = 10;

% new columns from data_gathering.ino (required)
dP = data.Pressure_dPaPerSec;
accel_dev = abs(data.AccelMag_mg - 1000);
imu_flag = data.IMU_Moving > 0;

% Learn jitter threshold without labels:
% choose t that best separates low-variance (still) and high-variance (moving)
valid_std = p_std(isfinite(p_std));
candidate_thresholds = linspace(prctile(valid_std, 5), prctile(valid_std, 95), 300);
best_score = -inf;
jitter_threshold = median(valid_std);

for t = candidate_thresholds
    low_group = valid_std(valid_std <= t);
    high_group = valid_std(valid_std > t);

    if isempty(low_group) || isempty(high_group)
        continue;
    end

    w0 = numel(low_group) / numel(valid_std);
    w1 = numel(high_group) / numel(valid_std);
    mu0 = mean(low_group);
    mu1 = mean(high_group);

    between_class_variance = w0 * w1 * (mu0 - mu1)^2;

    if between_class_variance > best_score
        best_score = between_class_variance;
        jitter_threshold = t;
    end
end

still_ratio = mean(valid_std < jitter_threshold);

% derivative threshold from data itself
valid_dp = abs(dP(isfinite(dP)));
derivative_threshold = prctile(valid_dp, 70);

accel_threshold = prctile(accel_dev(isfinite(accel_dev)), 70);

fprintf('--- LEARNED THRESHOLDS (UNSUPERVISED) ---\n');
fprintf('JITTER_THRESHOLD = %.4f Pa\n', jitter_threshold);
fprintf('DERIVATIVE_THRESHOLD = %.4f Pa/s\n', derivative_threshold);
fprintf('ACCEL_DEVIATION_THRESHOLD = %.2f mg\n', accel_threshold);
fprintf('Still ratio below std threshold = %.3f\n', still_ratio);

% still candidate if pressure window is flat AND derivative small.
is_still_std = p_std < jitter_threshold;
is_still_dp = abs(dP) < derivative_threshold;
is_still_acc = accel_dev < accel_threshold;

% if IMU says moving, force it to moving.
% //<-- this is for noisy pressure where imu catches movement better
is_green = is_still_std & is_still_dp & is_still_acc & ~imu_flag;

% K-style clustering. looking for <x_floors>*2 <-- random number i chose
% idk tbh
% amount of floors. k clustering
% should have some spare incase balls don't land properly
x_floors = 7;

if nnz(is_green) < 2 * x_floors
    error('Not enough still samples after filtering. Try lower thresholds or collect longer run.');
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

% vis
figure;
plot(P, 'Color', [0.8 0.8 0.8]); hold on;

% make green where good data
plot(find(is_green), P(is_green), 'g.', 'MarkerSize', 5);
yline(unique_thetas, 'r-', 'Learned Floors', 'LineWidth', 2);
title(['Detected ', num2str(length(unique_thetas)), ' Unique Floors']);
grid on;

% quick verify movement against derivative + imu
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

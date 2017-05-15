f1 = csvread ("../polar_simu_clean.csv");
%f1 = f1(2:end,1:end-1);

% f = left_speed,right_speed,left_command,right_command,  *2
% time,left_command,right_command,robot_speed.distance,speed_order.distance,

function color = speed2color(speed)
  if (speed >= 0)
    color = "r";
    %color = [speed, 0, 0];
  else
    color = "b"; 
    %color = [0, 0, speed];
  endif
endfunction

%1:pose_order_x,pose_order_y,pose_order_a,
%4:pose_current_x,pose_current_y,pose_current_a,
%7:position_error_l,position_error_a,
%9:speed_order_l,speed_order_a,speed_current_l,speed_current_a,

figure(1)
clf;
%x = f1(:, 1);
%y = f1(:, [4,5]);
x = f1(2:end,4);
y = f1(2:end,5);

t = f1(2:end,13);

%%%plot(x,y*10);
c = f1(2:end,10);

pose_order_a = f1(2:end,3);
pose_current_a = f1(2:end,6);
speed_current_l = f1(2:end,11);

%[col] = arrayfun(@speed2color, c, "UniformOutput", false);
%col(:,1)
col = arrayfun(@speed2color, c);
%plot(x,y, "color", "r");
%scatter(x,y,1,col);
scatter(x,y,4,speed_current_l,"filled");
%plot(x,y, "color", col);

hold on;

% second plot: vectors for current speed + angle:

% sub-matrix of point for each seconds (20ms sched)
sub_x = x(1:25:end);
sub_y = y(1:25:end);

sub_a = pose_current_a(1:25:end);
sub_a = sub_a * 2*pi / 360; % deg2rad

sub_speed_cur = speed_current_l(1:25:end);

%sub_arrow_end_x = sub_x + cos(sub_a);% * 1/sub_speed_cur;
%sub_arrow_end_y = sub_y + sin(sub_a);% * 1/sub_speed_cur;
sub_u = cos(sub_a); % .* sub_speed_cur;
sub_v = sin(sub_a); % .* sub_speed_cur;

% some arrow to display bot orientation
%quiver(sub_x, sub_y, sub_u, sub_v); hold on;

%plot(sub_x, sub_y, "color", "b");

%plot(x,y, "color", speed2color(c));

title ("Position controller");
xlabel ("x [mm]");
ylabel ("y [mm]");
%legend('robot speed','speed order')
grid;
axis equal;


%
%figure(2)
%clf;

%plot(t,pose_order_a, "color", "g");
%hold on;
%plot(t,pose_current_a , "color", "b");

%grid;
%axis equal;

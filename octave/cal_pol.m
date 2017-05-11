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

figure(1)
%x = f1(:, 1);
%y = f1(:, [4,5]);
x = f1(:,4);
y = f1(:,5);
%%%plot(x,y*10);
c = f1(:,10);
%[col] = arrayfun(@speed2color, c, "UniformOutput", false);
%col(:,1)
col = arrayfun(@speed2color, c);
%plot(x,y, "color", "r");
plot(x,y, "color", col);
%plot(x,y, "color", speed2color(c));
%quiver(x,y,c,c);
title ("Position controller");
xlabel ("x [mm]");
ylabel ("y [mm]");
%legend('robot speed','speed order')
grid;

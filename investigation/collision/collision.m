close all;
figure;
hold on;
grid on;
axis equal;
view(30, 30)

vertex = [625 500 -10;
          675 500 -10;
          675,500, 10;
          625,500, 10];

indices = [ 0 1 2
            2 3 0];

vector  = [ 640, 480, 5;
            640, 520, 5 ];

%a = vertex(indices(1)+1,:);
%b = vertex(indices(2)+1,:);
%c = vertex(indices(3)+1,:);

a = vertex(indices(4)+1,:);
b = vertex(indices(5)+1,:);
c = vertex(indices(6)+1,:);

ab = b - a;
ac = c - a;

bc = c - b;
ca = a - c;

n = cross(ab,ac);

s = vector(1,:);
e = vector(2,:);

t = -1 * dot(n,s-a) / dot(n,e-s);

if 0 <= t && t <= 1

  p = s + t*(e - s);

  ap = p - a;
  c1 = cross(ab,ap);
  check1 = dot(c1,n);


  bp = p - b;
  c2 = cross(bc,bp);
  check2 = dot(c2,n);

  cp = p - c;
  c3 = cross(ca,cp);
  check3 = dot(c3,n);

  if (check1 > 0 && check2 > 0 && check3 > 0) || (check1 < 0 && check2 < 0 && check3 < 0)
    disp("Collision!!");
  else
    disp("No collision...");
  end
else
    disp("No collision...");
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
x = [ vertex(indices(1)+1,1)
      vertex(indices(2)+1,1)
      vertex(indices(3)+1,1) ];

y = [ vertex(indices(1)+1,2)
      vertex(indices(2)+1,2)
      vertex(indices(3)+1,2) ];

z = [ vertex(indices(1)+1,3)
      vertex(indices(2)+1,3)
      vertex(indices(3)+1,3) ];

fill3(x, y, z, 'r');

x = [ vertex(indices(4)+1,1)
      vertex(indices(5)+1,1)
      vertex(indices(6)+1,1) ];

y = [ vertex(indices(4)+1,2)
      vertex(indices(5)+1,2)
      vertex(indices(6)+1,2) ];

z = [ vertex(indices(4)+1,3)
      vertex(indices(5)+1,3)
      vertex(indices(6)+1,3) ];

fill3(x, y, z, 'y');

x = [vector(1,1) vector(2,1)];
y = [vector(1,2) vector(2,2)];
z = [vector(1,3) vector(2,3)];

plot3(x,y,z);

xlabel("x");
ylabel("y");
zlabel("z");
grid on;

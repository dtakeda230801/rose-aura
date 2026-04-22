close all

Y = dlmread("C:/works/dev/github/rose-aura/test/rose_aura_test/testResultY.csv",",");
U = dlmread("C:/works/dev/github/rose-aura/test/rose_aura_test/testResultU.csv",",");
V = dlmread("C:/works/dev/github/rose-aura/test/rose_aura_test/testResultV.csv",",");

figure;
imagesc(Y);
colormap(jet);
colorbar;

figure;
imagesc(U);
colormap(jet);
colorbar;

figure;
imagesc(V);
colormap(jet);
colorbar;

[h,w] = size(U);

for y = 1 : h
  for x = 1 : w
    Us(y,(x - 1)*2 + 1)  = U(y,x);
    Us(y,(x - 1)*2 + 2)  = U(y,x);

    Vs(y,(x - 1)*2 + 1)  = V(y,x);
    Vs(y,(x - 1)*2 + 2)  = V(y,x);
  endfor
endfor

_Y = Y / 256;
_U = (Us - 128) / 128;
_V = (Vs - 128) / 128;


R = _Y + (1.5748 * _V);
G = _Y - (0.1873 * _U) - (0.4681 * _V);
B = _Y + (1.8556 * _U);

[h,w] = size(R);

figure;

RGB = cat(3, R, G, B);
image(RGB);
axis equal tight;



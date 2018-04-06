clear all
filename = 'iXon_img32a.csv';

oldpath = path;
path(oldpath, 'C:\Users\KRbG2\Desktop\Kyle\andor\data\20180405');

img = csvread(filename);

dims = size(img);
xdim = [1:dims(1)];
ydim = [1:dims(2)];

imagesc(xdim, ydim, img)
shg
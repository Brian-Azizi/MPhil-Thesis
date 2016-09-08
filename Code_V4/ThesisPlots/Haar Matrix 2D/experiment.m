% close all;
% % x = [2;4;2;1;0;6;1;2];
% x = [2;4;2;1];
% bar(x);
% axis([0,5,-5,5]);
% 
% [yl,yh] = dwt(x,'haar');
% y = [yl;yh];
% hold on;
% stairs([yl;yh]);
% 
% W = [1,1,0,0;0,0,1,1;1,-1,0,0;0,0,1,-1]/sqrt(2);
% 
% h1 = [1/sqrt(2) 1/sqrt(2) 0 0]; %h(x)
% h2 = [0 0 1/sqrt(2) 1/sqrt(2)]; 
% g1 = [1/sqrt(2) -1/sqrt(2) 0 0];
% g2 = [0 0 1/sqrt(2) -1/sqrt(2)];
% 
% W2 = [h1(:) h2(:) g1(:) g2(:)];
% 

close all;

x = magic(4);
figure, colormap(gray),imagesc(x);
[ya,yh,yv,yd] = dwt2(x,'haar');
y = [ya,yv;yh,yd];
figure, colormap(gray),imagesc(y);

W = [1,1,0,0;0,0,1,1;1,-1,0,0;0,0,1,-1]/sqrt(2);
W = kron(W,W);
[y(:), W*x(:)] % Compare matlab DWT vs DWT using kronecker product

% a11 = [1 1 0 0; 1 1 0 0; 0 0 0 0; 0 0 0 0]/2;
% a12 = [0 0 1 1; 0 0 1 1; 0 0 0 0; 0 0 0 0]/2;
% a21 = [0 0 0 0; 0 0 0 0; 1 1 0 0; 1 1 0 0]/2;
% a22 = [0 0 0 0; 0 0 0 0; 0 0 1 1; 0 0 1 1]/2;
% v11 = [1 -1 0 0; 1 -1 0 0; 0 0 0 0; 0 0 0 0]/2;
% v12 = [0 0 1 -1; 0 0 1 -1; 0 0 0 0; 0 0 0 0]/2;
% v21 = [0 0 0 0; 0 0 0 0; 1 -1 0 0; 1 -1 0 0]/2;
% v22 = [0 0 0 0; 0 0 0 0; 0 0 1 -1; 0 0 1 -1]/2;
% h11 = [1 1 0 0; -1 -1 0 0; 0 0 0 0; 0 0 0 0]/2;
% h12 = [0 0 1 1; 0 0 -1 -1; 0 0 0 0; 0 0 0 0]/2;
% h21 = [0 0 0 0; 0 0 0 0; 1 1 0 0; -1 -1 0 0]/2;
% h22 = [0 0 0 0; 0 0 0 0; 0 0 1 1; 0 0 -1 -1]/2;
% d11 = [1 -1 0 0; -1 1 0 0; 0 0 0 0; 0 0 0 0]/2;
% d12 = [0 0 1 -1; 0 0 -1 1; 0 0 0 0; 0 0 0 0]/2;
% d21 = [0 0 0 0; 0 0 0 0; 1 -1 0 0; -1 1 0 0]/2;
% d22 = [0 0 0 0; 0 0 0 0; 0 0 1 -1; 0 0 -1 1]/2;
% 
% W2 = [a11(:),a12(:),a21(:),a22(:),v11(:),v12(:),v21(:),v22(:),h11(:),h12(:),h21(:),h22(:),d11(:),d12(:),d21(:),d22(:)];
% 
% la = ones(4);
% lv = [ones(4,2), -ones(4,2)];
% lh = [ones(2,4); -ones(2,4)];
% ld = [ones(2,2), -ones(2,2); -ones(2,2), ones(2,2)];
% 
% WW = [la(:),lv(:),lh(:),ld(:),v11(:),v12(:),v21(:),v22(:),h11(:),h12(:),h21(:),h22(:),d11(:),d12(:),d21(:),d22(:)];
% 

W2 = haarMatrix2D(4,1);

z = W2'*reshape(x',16,1);

[y(:), z] % compare matlab DWT vs DWT using directly produced matrix.
%z=reshape(z,4,4);
W1 = W';
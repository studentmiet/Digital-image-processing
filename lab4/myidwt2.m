function [x] = myidwt2(a,h,v,d)
% ���������� �������������� ��������� ���-�������������� bior4.4
% � �������� ����������� ����������� ��� ���������� (����������)
% ���� ����� � �������� �� �����, �������������� ����� ������������ 
% ��������� ����������.
% 
% �����: ���������� ����� � �������� � 
% ������� � ������ ���� ������!

dwtmode('symw','nodisp');
dim = size(a); % �������� ������� x

Aa = zeros(dim+4);
Aa(3:dim(1)+2, 3:dim(2)+2) = a; 
Aa(1:2,:)=Aa(5:-1:4,:); Aa(dim(1)+3 : dim(1)+4,:)=Aa(dim(1)+2 :-1: dim(1)+1,:);
Aa(:,1:2)=Aa(:,5:-1:4); Aa(:,dim(2)+3 : dim(2)+4)=Aa(:,dim(2)+2 :-1: dim(2)+1);

Hh = zeros(dim+4);
Hh(3 : dim(1)+2, 3 : dim(2)+2) = h; 
Hh(1:2,:)=Hh(4:-1:3,:); Hh(dim(1)+3 : dim(1)+4,:)=Hh(dim(1)+1 :-1:dim(1),:);
Hh(:,1:2)=Hh(:,5:-1:4); Hh(:,dim(2)+3 :dim(2)+4)=Hh(:,dim(2)+2 :-1: dim(2)+1);

Vv = zeros(dim+4);
Vv(3:dim(1)+2, 3:dim(2)+2) = v; 
Vv(1:2,:)=Vv(5:-1:4,:); Vv(dim(1)+3 :dim(1)+4,:)=Vv(dim(1)+2:-1:dim(1)+1,:);
Vv(:,1:2)=Vv(:,4:-1:3); Vv(:,dim(2)+3:dim(2)+4)=Vv(:,dim(2)+1:-1:dim(2));

Dd = zeros(dim+4);
Dd(3:dim(1)+2, 3:dim(2)+2) = d; 
Dd(1:2,:)=Dd(4:-1:3,:); Dd((dim(1)+3):(dim(1)+4),:)=Dd((dim(1)+1):-1:dim(1),:);
Dd(:,1:2)=Dd(:,4:-1:3); Dd(:,(dim(2)+3):(dim(2)+4))=Dd(:,(dim(2)+1):-1:dim(2));

x = idwt2(Aa,Hh,Vv,Dd,'bior4.4');
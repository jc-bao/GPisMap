%
% GPisMap - Online Continuous Mapping using Gaussian Process Implicit Surfaces
% https://github.com/leebhoram/GPisMap
%
% This program is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License v3 as published by
% the Free Software Foundation.
%
% This program is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of any FITNESS FOR A
% PARTICULAR PURPOSE. See the GNU General Public License v3 for more details.
%
% You should have received a copy of the GNU General Public License v3
% along with this program; if not, you can access it online at
% http://www.gnu.org/licenses/gpl-3.0.html.
%
% Authors: Bhoram Lee <bhoram.lee@gmail.com>
%

close all
clearvars

addpath('./plot_scripts');
addpath('../mex');
mexGPisMap3('reset')

% The original dataset downloadable at http://rll.berkeley.edu/bigbird/
% The following data are sampled and prepared by the authors for the test
% depthpath = '../data/3D/bigbird_detergent/masked_depth';
% poses = single(load('../data/3D/bigbird_detergent/pose/poses.txt'));

% input sequence
% FrameNums = [93:3:359 3:3:90];  % numel : 120
% CamIDs = repmat([1 2 3 4 3 2],1,30);

% test 3D volume grid
[xg, yg, zg ] = meshgrid(-0.04:0.02:0.04, -0.04:0.02:0.04, -0.04:0.02:0.04);
xtest1 = single([xg(:)'; yg(:)'; zg(:)']);

load('../data/3D/carved_cube/data.mat')

n = size(depth, 1);
% n = numel(FrameNums);

count = 0;
for k=1:n
%     frmNo = FrameNums(k);
%     count = count + 1;
%     camID = CamIDs(count);
    camID = 0;

%     D = imread(fullfile(depthpath,sprintf('frame%d_cam%d.png',frmNo,camID)));
%     D = single(D)*single(0.0001); % 10 mm to meter
    D = single(squeeze(depth(k,:,:)));

%     T = reshape(poses(count,:),4,4);
    T = single(squeeze(pose(k,:,:)));

    R = T(1:3,1:3);
    t = T(4,1:3)';

    mexGPisMap3('setCamera',camID,'cube'); % See mex/mexGPisMap3.cpp for camera calibration info
%     mexGPisMap3('setCamera',camID,'bigbird');
    mexGPisMap3('update',D,[t' reshape(R,1,[])]);

    close all;
    if 1 % k == n
       visualize_gpisMap3
    end

    % paus if needed
    % disp('press a button to continue');
    % pause;

end

% clear resources
mexGPisMap3('reset')

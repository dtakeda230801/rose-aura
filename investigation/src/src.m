clear;
close all;
pkg load signal

%%%
[x, fs] = audioread("test.wav");

if columns(x) == 2
  x = mean(x, 2);
endif

N = length(x);

%%%
clear rate_converter

dst_fs = 48000;

window_size  = 1024;
window_start = 1;
window_end   = window_start + window_size - 1;

src_out = [];

while window_start < N
  y = x(window_start:window_end);

  src_out_temp = rate_converter( y, 1/fs , 1 / dst_fs );
  src_out_len = length(src_out);
  src_out(src_out_len + 1 : src_out_len + length(src_out_temp),1) = src_out_temp;

  window_start = window_start + window_size;
  window_end   = window_start + window_size - 1;
  if window_end > N
    window_end = N;
  endif
endwhile

size(src_out)

custom_plot(x      ,441000)
custom_plot(src_out,dst_fs)



clear;
close all;
pkg load signal;
warning("off");

%%%
[in, fs] = audioread("test.wav");

if columns(in) == 2
  in = mean(in, 2);
endif

in_len = length(in);

%%%
clear rate_converter

dst_fs = 48000;

window_size  = 1024;
window_start = 1;
window_end   = window_start + window_size - 1;

out = [];

while window_start < in_len
  in_with_win = in(window_start:window_end);

  out_temp = rate_converter( in_with_win, 1/fs , 1 / dst_fs );
  out_len = length(out);
  out(out_len + 1 : out_len + length(out_temp),1) = out_temp;

  window_start = window_start + window_size;
  window_end   = window_start + window_size - 1;
  if window_end > in_len
    window_end = in_len;
  endif
endwhile

printf("sample num %d -> %d\n",in_len,length(out))

custom_plot(in,fs)
custom_plot(out,dst_fs)



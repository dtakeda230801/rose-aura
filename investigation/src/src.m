clear;
close all;

function dst = src(in,src_fs,dst_fs)

  len = length(in);

  dst_count = 0;

  for i = 0:len-1
    t_in          = i * (1/src_fs);
    t_in_plus_one = (i+1)*(1/src_fs);

    if (t_in <= dst_count * (1/dst_fs)
     && dst_count * (1/dst_fs) < t_in_plus_one)
     dst(dst_count+1) = (src(i+2) - src(i+1)) * (dst_count * (1/dst_fs) - t_in) + src(i+1);
    endif
  endfor
end

pkg load signal

[x, fs] = audioread("test.wav");

if columns(x) == 2
  x = mean(x, 2);
endif

N = length(x);

N_mod = mod(N,200)

dst_pos = 1;
dst

for w = 1:199:N - N_mod
  y = x(w:w+199);
  dst = src(y,fs,10*000);
  dst_ret(dst_pos:length(dst)) = dst;
endfor
y = x(w:w+N_mod-1);
length(y)

t = (0:N-1) / fs;

figure(1);
plot(t, x);
xlabel("Time [sec]");
ylabel("Amplitude");
title("Waveform");
grid on;

X = fft(x);

P2 = abs(X / N);
P1 = P2(1:floor(N/2)+1);
P1(2:end-1) = 2 * P1(2:end-1);

f = fs * (0:floor(N/2)) / N;

figure(2);
semilogx(f, 20*log10(P1 + 1e-12));
xlabel("Frequency [Hz]");
ylabel("Magnitude [dB]");
title("Frequency Spectrum (Log Frequency)");
grid on;
xlim([20 fs/2]);

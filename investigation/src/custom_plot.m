function custom_plot(in,fs)

  in_len = length(in);

  t_scale = (0:in_len-1) / fs;

  figure();
  plot(t_scale, in);
  xlabel("Time [sec]");
  ylabel("Amplitude");
  title("Waveform");
  grid on;

  in_f = fft(in);

  P2 = abs(in_f / in_len);
  P1 = P2(1:floor(in_len/2)+1);
  P1(2:end-1) = 2 * P1(2:end-1);

  f_scale = fs * (0:floor(in_len/2)) / in_len;

  figure();
  semilogx(f_scale, 20*log10(P1 + 1e-12));
  xlabel("Frequency [Hz]");
  ylabel("Magnitude [dB]");
  title("Frequency Spectrum (Log Frequency)");
  grid on;
  xlim([20 fs/2]);

end


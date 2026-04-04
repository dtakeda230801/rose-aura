function dst = rate_converter(in, src_period, dst_period)

  persistent dst_point
  persistent src_point
  persistent src_final_sample

  if isempty(dst_point)
    dst_point = 0;
  endif

  if isempty(src_point)
    src_point = 0;
  endif

  if isempty(src_final_sample)
    src_final_sample = [];
  endif

  _in = [ src_final_sample(:); in(:) ];
  len = length(_in);

  dst_count = 1;
  dst = [];

  for i = 1 : len - 2
    t_src          = src_point     * src_period;
    t_src_plus_one = (src_point+1) * src_period;


    while 1
      t_dst = dst_point * dst_period;
      if (t_src <= t_dst && t_dst  < t_src_plus_one)
        dst(dst_count,1) = (_in(i+1) - _in(i)) * (t_dst - t_src) + _in(i);
        dst_point = dst_point + 1;
        dst_count = dst_count + 1;
      else
        break
      endif
    endwhile
    src_point = src_point + 1;
  endfor

  src_point        = src_point - 1;
  src_final_sample = _in(length(_in));
end

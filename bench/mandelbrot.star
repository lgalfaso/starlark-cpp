def draw_mandelbrot(width, height, max_iter):
    gradient = " .:-=+*#%@"
    grad_len = len(gradient) - 1

    # Complex plane bounds
    re_min = -2.0
    re_max = 1.0
    im_min = -1.2
    im_max = 1.2
    for y in range(height):
        row = ""
        c_im = im_max - (y * (im_max - im_min) / height)
        for x in range(width):
            c_re = re_min + (x * (re_max - re_min) / width)

            z_re = 0.0
            z_im = 0.0
            i = 0

            for i in range(max_iter):
                # z = z² + c
                zr2 = z_re * z_re
                zi2 = z_im * z_im

                if zr2 + zi2 > 4.0:
                    break

                z_im = 2.0 * z_re * z_im + c_im
                z_re = zr2 - zi2 + c_re

            if i == max_iter - 1:
                # Inside the set
                row += gradient[-1]
            else:
                # Smooth gradient mapping
                idx = (i * grad_len) // max_iter
                row += gradient[idx]

        print(row)

draw_mandelbrot(240, 80, 50)

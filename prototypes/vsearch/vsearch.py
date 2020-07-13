import imageio
import numpy as np
import struct
import math
import os

lines_searched = 0

def ray_from_centre(image, angle):
    global lines_searched
    lines_searched += 1

    image_height = len(image)
    image_width = len(image[0])

    centre_y = image_height / 2
    centre_x = image_width / 2

    rise = math.sin(angle)
    run = math.cos(angle)

    t_min = 0
    t_max = min(abs(0.5*(image_height - 1)/rise), abs(0.5*(image_width - 1)/run))

    def pixel_along_line_in_hrz(t):
        x = centre_x + run * t
        y = centre_y + rise * t

        # centre of pixel is (.5, .5)
        x_index = round(x - 0.5)
        y_index = round(image_height - (y + 0.5))

        return image[y_index][x_index] > 0

    min_in_hrz = pixel_along_line_in_hrz(t_min)
    max_in_hrz = pixel_along_line_in_hrz(t_max)

    if min_in_hrz == max_in_hrz:
        # This means there is no edge along the line
        return False, 0

    epsilon = 0.5

    while abs(t_min - t_max) > epsilon:

        t_mid = 0.5 * (t_min + t_max)

        if pixel_along_line_in_hrz(t_mid) == min_in_hrz:
            # This means hrz is between mid and max
            t_min = t_mid
        else:
            # This means hrz is between min and mid
            t_max = t_mid

    if min_in_hrz:
        # This means the camera is pointing below the horizon
        return True, -0.5 * (t_min + t_max)
    else:
        # This means the camera is pointing above the horizon
        return True, 0.5 * (t_min + t_max)


def main():
    image_folder = '../test_images/images'
    data_folder =  '../test_images/metadata'

    images = np.sort(os.listdir(image_folder))
    metadata = np.sort(os.listdir(data_folder))

    test_results = []

    for im, da in zip(images, metadata):
        image = imageio.imread(image_folder + "/" + im)

        # Find the vertex with the bisection method.
        # To do this, an angle pointing at the horizon on either side of the vertex is needed.
        test_angle = 0

        found_hz = True

        while found_hz:
            test_angle += 0.05
            found_hz, d = ray_from_centre(image, test_angle)

        while not found_hz:
            test_angle += 0.05
            found_hz, d = ray_from_centre(image, test_angle)

        above_hz = (d >= 0)
        hz_start_angle = test_angle

        while found_hz:
            test_angle += 0.05
            found_hz, d = ray_from_centre(image, test_angle)
        
        hz_end_angle = test_angle - 0.05

        if hz_end_angle >=  math.pi:
            hz_start_angle -= 2 * math.pi
            hz_end_angle -= 2 * math.pi

        found_hz, dstart = ray_from_centre(image, hz_start_angle)
        assert(found_hz)
        found_hz, dend = ray_from_centre(image, hz_end_angle)
        assert(found_hz)

        # Take the smallest d, so that we can find a symmetric point on the edge

        if abs(dstart) > abs(dend):
            target_d = dend
            base_angle = hz_end_angle
            a2 = hz_start_angle
            d2 = dstart
        else:
            target_d = dstart
            base_angle = hz_start_angle
            a2 = hz_end_angle
            d2 = dend

        a1 = 0.5 * (base_angle + a2)
        found_hz, d1 = ray_from_centre(image, a1)

        # TODO: These aren't true in some edge cases
        assert(found_hz)
        assert(abs(d1) < abs(target_d))
     
        epsilon = 0.01
        while abs(a1 - a2) > epsilon:
            test_angle = 0.5 * (a1 + a2)
            found_hz, test_d = ray_from_centre(image, test_angle)

            # TODO: This isn't true in some edge cases
            assert(found_hz)

            if abs(test_d) < abs(target_d):
                a1 = test_angle
            elif abs(test_d) > abs(target_d):
                a2 = test_angle
            else:
                break

        symmetric_angle = 0.5 * (a1 + a2)
        found_hz, d = ray_from_centre(image, symmetric_angle)
        assert(found_hz)

        angle_to_vertex = 0.5 * (base_angle + symmetric_angle)
        found_hz, vertex_d = ray_from_centre(image, angle_to_vertex)
        assert(found_hz)

        angle_above_hz = math.atan((vertex_d / (0.5 * len(image[0]))) * math.tan(0.5 * 57 * math.pi / 180))

        earth_angular_radius = math.asin(6371.0 / 6871.0)

        theta_x = earth_angular_radius + angle_above_hz

        if angle_above_hz > 0:
            theta_z = angle_to_vertex + 0.5 * math.pi
        else:
            theta_z = angle_to_vertex - 0.5 * math.pi

        nadir = np.matrix([0, 0, -1]).T

        th_x = -theta_x;
        th_z = theta_z;

        Rx = np.matrix([[1, 0, 0],
                        [0, math.cos(th_x), -1*math.sin(th_x)],
                        [0, math.sin(th_x), math.cos(th_x)]])
                    
        Rz = np.matrix([[math.cos(th_z), -1*math.sin(th_z), 0],
                        [math.sin(th_z), math.cos(th_z), 0],
                        [0, 0, 1]])

        nadir = Rz * Rx * nadir;

        with open(data_folder + '/' + da, "rb") as f:
            chunk = f.read(44)
                        
        m = struct.unpack('ffiifffffff',chunk)

        nadir_actual = np.matrix([m[8], m[9], m[10]])
        angle_error = math.acos(nadir_actual * nadir)
        print("Error: " + str(angle_error * 180 / math.pi) + " degrees")
        print("Lines searched: " + str(lines_searched))

if __name__ == "__main__":
    main()

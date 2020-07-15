SELECT d.drawing_number AS drawing_number, d.width AS width, d.length AS length, mal.aperture_id AS aperture_id FROM drawings AS d
INNER JOIN mat_aperture_link AS mal on d.mat_id = mal.mat_id
WHERE d.mat_id=6936;
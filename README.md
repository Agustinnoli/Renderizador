# Renderizador 3D por Software

Implementación de un renderizador 3D escrito en C utilizando SDL2, desarrollado con fines educativos para explorar los fundamentos de la computación gráfica y el funcionamiento interno de un pipeline gráfico moderno sin depender de OpenGL, DirectX o motores externos.

## Tecnologías

- C
- SDL2

## Conceptos Implementados

### Pipeline Gráfico

El proyecto implementa manualmente las etapas fundamentales de un pipeline de renderizado:

1. Transformación de vértices.
2. Posicionamiento de cámara.
3. Clipping geométrico.
4. Proyección perspectiva.
5. Conversión a coordenadas de pantalla.
6. Rasterización de primitivas.

### Computación Gráfica

- Álgebra lineal aplicada a gráficos.
- Transformaciones tridimensionales.
- Sistemas de coordenadas.
- Frustum culling.
- Clipping de polígonos.
- Renderizado por software.

## Controles

| Tecla | Acción |
|---------|---------|
| W / S | Acercar / Alejar cámara |
| A / D | Mover cámara horizontalmente |
| Space / Shift | Mover cámara verticalmente |
| ← / → | Rotar escena |

## Objetivo

El objetivo principal del proyecto fue comprender cómo funcionan internamente los motores gráficos y los pipelines de renderizado, implementando manualmente cada etapa fundamental utilizando únicamente C y SDL2.

## Capturas

## Estado actual

Actualmente el proyecto implementa:

- Transformaciones 3D
- Rotaciones y traslaciones
- Proyección en perspectiva
- Clipping contra el near plane
- Renderizado wireframe
- Navegación interactiva mediante teclado

## Próximos pasos

El objetivo es evolucionar el renderer hacia un pipeline gráfico más completo:

- [ ] Rasterización de triángulos
- [ ] Z-Buffer (Depth Buffer)
- [ ] Back-face culling
- [ ] Iluminación difusa (Lambert)
- [ ] Iluminación especular (Phong)
- [ ] Texturas
- [ ] Carga de modelos OBJ
- [ ] Cámara libre
- [ ] Frustum clipping completo
- [ ] Optimizaciones de rendimiento

Agregar aquí imágenes o GIFs mostrando el renderizador en funcionamiento.

## Aprendizajes

Durante el desarrollo de este proyecto se trabajó con:

- Programación de bajo nivel en C.
- Manejo manual de memoria.
- Estructuras matemáticas para gráficos 3D.
- Algoritmos de clipping.
- Proyección perspectiva.
- Arquitectura básica de renderizadores por software.

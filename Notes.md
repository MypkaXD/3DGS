Какие есть решения в индустрии:
1. https://github.com/graphdeco-inria/gaussian-splatting - репозиторий автора статьи "3D Gaussian Splatting for Real-Time Radiance Field Rendering". Тут можно найти viewer (4GB VRAM) и самому обучить на основе картинок (24GB VRAM). + collab, который не работает xDD
2. https://github.com/MrNeRF/awesome-3D-gaussian-splatting - сборник ссылок про 3DGS (Tools, Utilities, Viewers, Learning Resourses)
3. https://guanjunwu.github.io/4dgs/?id=4DGaussianSplatting - 4D Gaussian Splatting for Real-Time Dynamic Scene Rendering
4. https://github.com/fbriggs/lifecast_public - проект по созданию видео 3D/4D GS (закрылись)
5. https://www.gracia.ai/ - Gaussian splatting-based volumetric videos for XR and beyond
6. https://www.jawset.com/ - 3DGS создание сцен с помощью камеры
7. https://www.reddit.com/r/GaussianSplatting/comments/1ll1s03/realtime_triangle_splatting_in_unity_now_with/ - ссылка на triangle splatting в Unity для рендера
8. https://80.lv/articles/animated-gaussian-splatting-in-unreal-engine-5 - плагин для 4DGS в Unreal Engine

Интересные находки:
1. Triangles Splatting - По сравнению с популярными методами 2D и 3D Gaussian Splatting, наш подход обеспечивает более высокую визуальную точность, более быструю сходимость и повышенную пропускную способность рендеринга. "The final output is immediately usable in standard graphics engines without postprocessing (в отличии от 3DGS) When its triangle soup is rendered in a game engine, a noticeable drop in visual quality occurs because training does not enforce opaque triangles but instead relies on soft, semi-transparent ones." (цитаты из статьи, пока что её читаю)
   - https://trianglesplatting.github.io/
   - https://trianglesplatting2.github.io/trianglesplatting2/
   - https://www.youtube.com/watch?v=F0H3NAHP9r0 (24 июня 2025 года, человек говорил, что нет viewer'a для triangles splatting + тут простенький запуск triangle splatting на своей машине)
   - https://arxiv.org/pdf/2509.25122 (Triangle Splatting+: Differentiable Rendering with Opaque Triangles - статья от 29 сентября 2025 года)
   - https://arxiv.org/pdf/2505.19175 (Triangle Splatting for Real-Time Radiance Field Rendering - статья от 25 мая 2025 года)
   - https://github.com/trianglesplatting/triangle-splatting
   - [[SurfaceSplatting.pdf]]

Отталкиваясь от них, что можно сделать своё:
1. Viewer (3D/4DGS)
2. Было бы интересно рассмотреть triangle splatting (если можно, но пока что непонятно что, может также viewer или написать полностью с нуля этот процесс (сложно, но я бы попробовал))
3. Плагин для Unity или UE, который позволит внедрить trinaglesSplatting или 3/4DGS. 
4. Один мужик на ютубе с помощью blendera сделал кучу картинок для модельки (рука с черепом, комната - 3DGS, бегущий бетмэн - 4DGS) и использовал их для обучения.
   - https://youtu.be/JsdWaF9bJiU?si=QogweLfEqNba3LI6 (4DGS)
   - https://www.youtube.com/watch?v=fr5xp9CAY5w (3DGS)


Статьи, которые полезно прочитать:
1. https://medium.com/data-science/a-comprehensive-overview-of-gaussian-splatting-e7d570081362 - подробный обзор GS (есть ссылки на репозитории, с которыми можно поиграться + ссылки на статьи, где можно что-то интересное почитать)
2. https://mrnerf.github.io/awesome-3D-gaussian-splatting/ - сборник "всех" статей про 3DGS (Awesome-3D-Gaussian-Splatting-Paper-List)
3. https://arxiv.org/pdf/2401.03890 !!! ТУТ ЕСТЬ ПРО ПРИМЕНЕНИЕ 3DGS (+ https://github.com/guikunchen/Awesome3DGS - статьи о применении 3DGS)

Вопросы:
1. Много комментариев людей о том, что это бесполезная технология, которая не найдет большого применения (3D/4DGS). Да и вообще вопрос про применимость данного метода (в VR, в киноиндустрии?)
2. Можно ли попробовать Triangle Splatting?


Обзоры статей:
1. Дифференциальный рендеринг - обзор
   Заметки [[Differentiable Rendering - A Survey]]
   Ссылка на статью - https://arxiv.org/abs/2006.12057 


Предложения:
1. Moushion Structured -> Point Cloud -> GetPrimitives
   KiriEngine
   Huawei в телефоне
   Khronos - point cloud + gaussians
   Гауссиан - объемыный элемент, задать треугольники - плоскость, а тут получается объемный элемент, а в зависимости от проекции он меняется ("переливается") -> Перспектив побольше
   Тема: Gaussian Splatting - модель освещения. Есть готовая сцена из гауссианов, добавляем им параметры, чтобы усложнить и добавить новые параметры (светы). Сможем моделировать поведение стекла (видеть поверхность за ним). В них есть прозрачность, но она херовая.
   Задача - научиться рендерить с использованием света (Лина-Фонга???, PVR???, Disneay Arnold???). Взять прототип (2023 основная статья, взять старые коммиты), потом изучить его и улучшить, доисследовать, чтобы построить модель освещения
   3DGS Ray Tracing (Path Tracing)
   Banchmarking???
   Моделировать смену времени (машина стоит, а солнце перемещается, прикольно впринципе)
   Сначала сделать сцену из гауссианов (можно взять из основной статьи), потом добавить параметры и получить пиздатое освещение.
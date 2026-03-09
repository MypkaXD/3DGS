# Программа для отрисовки многомерного Гауссова (нормального) распредлеления.

## Рассотрим одномерный случай $\mathbf{x} = \begin{pmatrix}x_1\end{pmatrix}$:

$f(x) = \frac{1.0}{\sqrt{2\pi}*\sigma}*e^{\frac{-(x-\mu)^2}{2\sigma^2}}$

$\mu=0.0$  
$\sigma=0.2$
![alt text](imgs/2d(mu=0,sigma=0.2).png)

$\mu=0.0$  
$\sigma=0.05$
![alt text](imgs/2d(mu=0,sigma=0.05).png)

## Рассотрим двумерный случай $\mathbf{x} = \begin{pmatrix} x_1 \\ x_2 \end{pmatrix}$:

$f(\mathbf{x}) = \frac{1}{2 \pi \sqrt{|\Sigma|}} \exp\Big(-\frac{1}{2} (\mathbf{x}-\boldsymbol{\mu})^T \Sigma^{-1} (\mathbf{x}-\boldsymbol{\mu})\Big)$

Так как ковариационная матрица диагональная, то

$f(x_1,x_2) = \frac{1}{2 \pi \sigma_1 \sigma_2} \exp\Big(-\frac{(x_1-\mu_1)^2}{2\sigma_1^2} - \frac{(x_2-\mu_2)^2}{2\sigma_2^2}\Big)$

$\mathbf{\mu}=\begin{pmatrix}0.0\\0.0\end{pmatrix}$  
  
$\mathbf{\sigma}=\begin{pmatrix}0.5\\0.5\end{pmatrix}$
![alt text](imgs/3d(mu=0%200,sigma=0.5%200.5).png)

$\mathbf{\mu}=\begin{pmatrix}0.0\\0.0\end{pmatrix}$  
  
$\mathbf{\sigma}=\begin{pmatrix}0.1\\0.5\end{pmatrix}$
![alt text](imgs/3d(mu=0%200%20sigma=0.1%200.5).png)

## Рассотрим трехмерный случай $\mathbf{x} = \begin{pmatrix} x_1 \\ x_2 \\ x_3 \end{pmatrix}$:

$\boldsymbol{\mu} = \begin{pmatrix} \mu_1 \\ \mu_2 \\ \mu_3 \end{pmatrix}$

$\Sigma = \begin{pmatrix} \sigma_1^2 & 0 & 0 \\ 0 & \sigma_2^2 & 0 \\ 0 & 0 & \sigma_3^2 \end{pmatrix}$

$f(\mathbf{x}) = \frac{1}{(2 \pi)^{3/2} \sqrt{|\Sigma|}} \exp\Big(-\frac{1}{2} (\mathbf{x}-\boldsymbol{\mu})^T \Sigma^{-1} (\mathbf{x}-\boldsymbol{\mu}) \Big)$

$f(x_1,x_2,x_3) = \frac{1}{(2 \pi)^{3/2} \sigma_1 \sigma_2 \sigma_3} \exp\Big(-\frac{(x_1-\mu_1)^2}{2\sigma_1^2} - \frac{(x_2-\mu_2)^2}{2\sigma_2^2} - \frac{(x_3-\mu_3)^2}{2\sigma_3^2} \Big)$

Для визуализации 4D пространства воспользуемся помимо координат $(x, y, z)$ размером точки.

При этом отнормируем значения $f(x_1,x_2,x_3)$ на интервал $[1.0; 2.0]$

$\boldsymbol{\mu} = \begin{pmatrix} 0.0 \\ 0.0 \\ 0.0 \end{pmatrix}$

$\boldsymbol{\sigma} = \begin{pmatrix} 0.1 \\ 0.1 \\ 0.7 \end{pmatrix}$

![alt text](imgs/4d(mu=0%200%200,sigma=0.1%200.1%200.7).png)

Из рисунка видно, что результат плохо заметен, поэтому визуализируем только те значения, которые лежат в интервале $[v_{target} - \epsilon; v_{target} + \epsilon]$

Рассмотрим $\epsilon=0.1; v_{target}=1.2$
![alt text](imgs/4d(mu=0%200%200,sigma=0.1%200.1%200.7)%20with%20eps.png)

$\boldsymbol{\mu} = \begin{pmatrix} 0.0 \\ 0.0 \\ 0.0 \end{pmatrix}$

$\boldsymbol{\sigma} = \begin{pmatrix} 0.5 \\ 0.7 \\ 0.9 \end{pmatrix}$

![alt text](imgs/4d(mu=0%200%200,sigma=0.5%200.7%200.9)%20with%20eps.png)
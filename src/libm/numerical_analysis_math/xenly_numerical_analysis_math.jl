#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Julia programming language.
#
# `xenly_numerical_analysis_math.jl` is the similar to the `xenly_numerical_analysis_math.c` in C programming language.
#
module xenly_numerical_analysis_math

export xenly_gmres, xenly_conjugate_gradient, xenly_gaussian_elimination, xenly_newtons_method, xenly_hill_climbing, xenly_bisection_method, xenly_eulers_method

# GMRES (Generalized Minimal RESidual method)
function xenly_gmres(A, b, x0, tol=1e-6, maxiter=100)
    n = length(b)
    Q = Matrix{Float64}(undef, n, maxiter + 1)
    H = zeros(Float64, maxiter + 1, maxiter)
    r = b - A*x0
    beta = norm(r)
    Q[:, 1] = r / beta

    for j = 1:maxiter
        v = A*Q[:, j]
        for i = 1:j
            H[i, j] = dot(Q[:, i], v)
            v -= H[i, j] * Q[:, i]
        end
        H[j+1, j] = norm(v)
        if H[j+1, j] < tol
            break
        end
        Q[:, j+1] = v / H[j+1, j]
        y = H[1:j, 1:j] \ (beta * I(j))
        x = x0 + Q[:, 1:j] * y
        r = b - A*x
        if norm(r) < tol
            return x, j
        end
    end

    return x, maxiter
end

# Conjugate gradient method
function xenly_conjugate_gradient(A, b, x0, tol=1e-6, maxiter=100)
    n = length(b)
    x = copy(x0)
    r = b - A * x
    p = copy(r)
    rsold = dot(r, r)

    for iter = 1:maxiter
        Ap = A * p
        alpha = rsold / dot(p, Ap)
        x += alpha * p
        r -= alpha * Ap
        rsnew = dot(r, r)
        if sqrt(rsnew) < tol
            return x, iter
        end
        p = r + (rsnew / rsold) * p
        rsold = rsnew
    end

    return x, maxiter
end

# Gaussian elimination
function xenly_gaussian_elimination(A, b)
    n = size(A, 1)
    Ab = hcat(A, b)

    for i = 1:n-1
        # Pivot selection (partial pivoting)
        maxval, maxrow = findmax(abs.(Ab[i:end, i]))
        maxrow += i - 1

        if maxval == 0
            error("Matrix is singular or nearly singular.")
        end

        if maxrow != i
            Ab[[i, maxrow], :] = Ab[[maxrow, i], :]
        end

        for j = i+1:n
            factor = Ab[j, i] / Ab[i, i]
            Ab[j, i:end] -= factor * Ab[i, i:end]
        end
    end

    x = zeros(n)
    x[n] = Ab[n, n+1] / Ab[n, n]

    for i = n-1:-1:1
        x[i] = (Ab[i, n+1] - dot(Ab[i, i+1:n], x[i+1:n])) / Ab[i, i]
    end

    return x
end

# Newton's method
function xenly_newtons_method(f, df, x0, tol=1e-6, maxiter=100)
    x = copy(x0)
    iter = 0
    while norm(f(x)) > tol && iter < maxiter
        x -= df(x) \ f(x)
        iter += 1
    end
    return x, iter
end

# Hill Climbing
function xenly_hill_climbing(f, x0; step_size=0.01, tol=1e-6, maxiter=1000)
    x = copy(x0)
    best_value = f(x)
    iter = 0
    while iter < maxiter
        candidate = x + step_size * (2 * rand(length(x)) .- 1)
        candidate_value = f(candidate)
        if candidate_value < best_value
            x = candidate
            best_value = candidate_value
        end
        iter += 1
        if norm(candidate - x) < tol
            break
        end
    end
    return x, best_value, iter
end

# Bisection method
function xenly_bisection_method(f, a, b; tol=1e-6, maxiter=1000)
    if f(a) * f(b) > 0
        error("f(a) and f(b) must have opposite signs")
    end

    fa, fb = f(a), f(b)
    iter = 0
    while (b - a) / 2 > tol && iter < maxiter
        c = (a + b) / 2
        fc = f(c)
        if fc == 0.0
            return c, iter
        elseif fa * fc < 0
            b = c
            fb = fc
        else
            a = c
            fa = fc
        end
        iter += 1
    end

    return (a + b) / 2, iter
end

# Euler's method
function xenly_eulers_method(f, y0, t0, t_end, h)
    t = t0:h:t_end
    y = zeros(length(t))
    y[1] = y0
    for i = 2:length(t)
        y[i] = y[i-1] + h * f(t[i-1], y[i-1])
    end
    return t, y
end

end # module
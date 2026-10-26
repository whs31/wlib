#pragma once

#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/concepts.h>
#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/precision.h>

#ifdef W_COMMON_WITH_QT
#  include <qstring.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Common implementation of the strongly typed scalar quantities.
 * @details Every WLib quantity, e.g. Distance, Mass or Voltage, derives from
 * `QuantityBase<Derived>` and stores a single magnitude of type `Scalar` in its SI unit. `Scalar`
 * defaults to Real, which follows the `precision` option of the library; a quantity that needs a
 * fixed precision passes its own. The base provides the operations that are the same for every
 * quantity:
 *   - `==` and `<=>` between equal types;
 *   - `+` and `-` between equal types, and `*` and `/` by a dimensionless factor;
 *   - `Derived / Derived`, which yields a dimensionless `Scalar` ratio;
 *   - is_finite(), abs(), zero() and the explicit conversion to the SI magnitude;
 *   - to_string(), `std::ostream` output, `fmt` formatting and `std::hash`;
 *   - the QuantityTraits specialization, so every derived type satisfies Quantity.
 *
 * A quantity of one type never mixes with a quantity of another type, and a quantity is never
 * multiplied by itself. Physically meaningful cross-type relations, e.g.
 * `Distance / std::chrono::duration -> Velocity`, are declared next to the types they relate.
 *
 * @par Layout
 * A derived type adds no data members, so it is standard layout and trivially copyable, and its
 * object representation is exactly one `Scalar` in the SI unit. The layout is part of the API.
 *
 * @par Deriving a quantity
 * A derived type declares a default constructor and an explicit constructor from the SI magnitude,
 * declares the symbol of its SI unit as `static constexpr std::string_view si_symbol`, and adds its
 * named factories and accessors. It may hide to_string() to choose another display unit.
 *
 * @code {.cpp}
 * class Luminance : public w::common::QuantityBase<Luminance>
 * {
 *  public:
 *   static constexpr std::string_view si_symbol = "cd/m^2";
 *
 *   constexpr Luminance() noexcept = default;
 *   explicit constexpr Luminance(w::Real nits) noexcept : QuantityBase(nits) {}
 *
 *   static constexpr Luminance from_nits(w::Real nits) noexcept { return Luminance(nits); }
 *   constexpr w::Real as_nits() const noexcept { return this->magnitude(); }
 * };
 * @endcode
 *
 * @tparam Derived The quantity type that derives from this base.
 * @tparam ScalarT Floating-point type of the magnitude.
 * @ingroup core_measurements
 * @see Quantity
 * @see QuantityTraits
 * @see Real
 */
template <typename Derived, std::floating_point ScalarT = Real>
class QuantityBase
{
 public:
  /**
   * @brief Floating-point type of the magnitude.
   */
  using Scalar = ScalarT;

  /**
   * @brief Returns a zero quantity.
   */
  [[nodiscard]] static constexpr Derived zero() noexcept { return Derived(); }

  /**
   * @brief Checks whether the magnitude is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_magnitude);
  }

  /**
   * @brief Returns the quantity with a nonnegative magnitude.
   */
  [[nodiscard]] constexpr Derived abs() const noexcept
  {
    return Derived(common::abs(this->m_magnitude));
  }

  /**
   * @brief Returns the magnitude formatted with the SI unit symbol, e.g. `"9.81 m/s^2"`.
   */
  [[nodiscard]] std::string to_string() const
  {
    return fmt::format("{:.6g} {}", this->m_magnitude, Derived::si_symbol);
  }

#ifdef W_COMMON_WITH_QT
  /**
   * @brief Returns to_string() of the derived type as a Qt string.
   * @note Available only when wlib is built with `with_qt=True`.
   */
  [[nodiscard]] QString to_qstring() const
  {
    return QString::fromStdString(this->derived().to_string());
  }
#endif

  [[nodiscard]] constexpr explicit operator float() const noexcept
  {
    return static_cast<float>(this->m_magnitude);
  }

  [[nodiscard]] constexpr explicit operator double() const noexcept
  {
    return static_cast<double>(this->m_magnitude);
  }

  [[nodiscard]] constexpr Derived operator+() const noexcept { return this->derived(); }

  [[nodiscard]] constexpr Derived operator-() const noexcept { return Derived(-this->m_magnitude); }

  constexpr Derived& operator+=(Derived const rhs) noexcept
  {
    this->m_magnitude += rhs.m_magnitude;
    return this->derived();
  }

  constexpr Derived& operator-=(Derived const rhs) noexcept
  {
    this->m_magnitude -= rhs.m_magnitude;
    return this->derived();
  }

  constexpr Derived& operator*=(Scalar const rhs) noexcept
  {
    this->m_magnitude *= rhs;
    return this->derived();
  }

  constexpr Derived& operator/=(Scalar const rhs) noexcept
  {
    this->m_magnitude /= rhs;
    return this->derived();
  }

  [[nodiscard]] friend constexpr bool operator==(Derived const lhs, Derived const rhs) noexcept
  {
    return lhs.m_magnitude == rhs.m_magnitude;
  }

  [[nodiscard]] friend constexpr std::partial_ordering operator<=>(
    Derived const lhs,
    Derived const rhs
  ) noexcept
  {
    return lhs.m_magnitude <=> rhs.m_magnitude;
  }

  [[nodiscard]] friend constexpr Derived operator+(Derived lhs, Derived const rhs) noexcept
  {
    lhs += rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr Derived operator-(Derived lhs, Derived const rhs) noexcept
  {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr Derived operator*(Derived lhs, Scalar const rhs) noexcept
  {
    lhs *= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr Derived operator*(Scalar const lhs, Derived rhs) noexcept
  {
    rhs *= lhs;
    return rhs;
  }

  [[nodiscard]] friend constexpr Derived operator/(Derived lhs, Scalar const rhs) noexcept
  {
    lhs /= rhs;
    return lhs;
  }

  /**
   * @brief Returns the dimensionless ratio of two quantities of the same type.
   */
  [[nodiscard]] friend constexpr Scalar operator/(Derived const lhs, Derived const rhs) noexcept
  {
    return lhs.m_magnitude / rhs.m_magnitude;
  }

  friend std::ostream& operator<<(std::ostream& os, Derived const& value)
  {
    return os << value.to_string();
  }

  /**
   * @brief Formats the quantity with `fmt` as its to_string().
   */
  [[nodiscard]] friend std::string format_as(Derived const& value) { return value.to_string(); }

 protected:
  /**
   * @brief Returns the magnitude in the SI unit.
   */
  [[nodiscard]] constexpr Scalar magnitude() const noexcept { return this->m_magnitude; }

 private:
  friend Derived;

  constexpr QuantityBase() noexcept = default;

  explicit constexpr QuantityBase(Scalar const si_magnitude) noexcept
    : m_magnitude(si_magnitude)
  {}

  [[nodiscard]] constexpr Derived& derived() noexcept { return static_cast<Derived&>(*this); }

  [[nodiscard]] constexpr Derived const& derived() const noexcept
  {
    return static_cast<Derived const&>(*this);
  }

  Scalar m_magnitude = 0.0;
};

/**
 * @brief Concept that is true if T derives from its own QuantityBase.
 * @tparam T Type to check.
 * @ingroup core_concepts
 * @see QuantityBase
 */
template <typename T>
concept DerivedQuantity = std::is_class_v<T> and requires { typename T::Scalar; }
                      and std::derived_from<T, QuantityBase<T, typename T::Scalar>>;

/**
 * @brief Quantity traits of every QuantityBase-derived type.
 * @details The SI symbol is the derived type's `si_symbol`.
 * @ingroup core_measurements
 * @see Quantity
 */
template <DerivedQuantity T>
struct QuantityTraits<T>
{
  using Scalar = typename T::Scalar;

  static constexpr std::string_view si_symbol = T::si_symbol;

  [[nodiscard]] static constexpr T from_si(Scalar const si_magnitude) noexcept
  {
    return T(si_magnitude);
  }
};

W_COMMON_NAMESPACE_END

// The partial specialization matches WLib quantities only, which the standard permits.
template <w::common::DerivedQuantity T>
struct std::hash<T>  // NOLINT(bugprone-std-namespace-modification)
{
  [[nodiscard]] std::size_t operator()(T const value) const noexcept
  {
    return std::hash<typename T::Scalar> {}(static_cast<typename T::Scalar>(value));
  }
};
